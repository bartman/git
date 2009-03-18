#include "cache.h"
#include "commit.h"
#include "tree-walk.h"
#include "attr.h"
#include "archive.h"
#include "parse-options.h"
#include "refs.h"

static char const * const archive_usage[] = {
	"git archive [options] <tree-ish> [path...]",
	"git archive --list",
	"git archive --remote <repo> [--exec <cmd>] [options] <tree-ish> [path...]",
	"git archive --remote <repo> [--exec <cmd>] --list",
	NULL
};

#define USES_ZLIB_COMPRESSION 1

static const struct archiver {
	const char *name;
	write_archive_fn_t write_archive;
	unsigned int flags;
} archivers[] = {
	{ "tar", write_tar_archive },
	{ "zip", write_zip_archive, USES_ZLIB_COMPRESSION },
};

static void format_subst(const struct commit *commit,
                         const char *src, size_t len,
                         struct strbuf *buf)
{
	char *to_free = NULL;
	struct strbuf fmt = STRBUF_INIT;

	if (src == buf->buf)
		to_free = strbuf_detach(buf, NULL);
	for (;;) {
		const char *b, *c;

		b = memmem(src, len, "$Format:", 8);
		if (!b)
			break;
		c = memchr(b + 8, '$', (src + len) - b - 8);
		if (!c)
			break;

		strbuf_reset(&fmt);
		strbuf_add(&fmt, b + 8, c - b - 8);

		strbuf_add(buf, src, b - src);
		format_commit_message(commit, fmt.buf, buf, DATE_NORMAL);
		len -= c + 1 - src;
		src  = c + 1;
	}
	strbuf_add(buf, src, len);
	strbuf_release(&fmt);
	free(to_free);
}

static void *sha1_file_to_archive(const char *path, const unsigned char *sha1,
		unsigned int mode, enum object_type *type,
		unsigned long *sizep, const struct commit *commit)
{
	void *buffer;

	buffer = read_sha1_file(sha1, type, sizep);
	if (buffer && S_ISREG(mode)) {
		struct strbuf buf = STRBUF_INIT;
		size_t size = 0;

		strbuf_attach(&buf, buffer, *sizep, *sizep + 1);
		convert_to_working_tree(path, buf.buf, buf.len, &buf);
		if (commit)
			format_subst(commit, buf.buf, buf.len, &buf);
		buffer = strbuf_detach(&buf, &size);
		*sizep = size;
	}

	return buffer;
}

static void setup_archive_check(struct git_attr_check *check)
{
	static struct git_attr *attr_export_ignore;
	static struct git_attr *attr_export_subst;

	if (!attr_export_ignore) {
		attr_export_ignore = git_attr("export-ignore", 13);
		attr_export_subst = git_attr("export-subst", 12);
	}
	check[0].attr = attr_export_ignore;
	check[1].attr = attr_export_subst;
}

static int include_repository(const char *path)
{
	struct stat st;
	const char *tmp;

	/* Return early if the path does not exist since it is OK to not
	 * checkout submodules.
	 */
	if (stat(path, &st) && errno == ENOENT)
		return 1;

	tmp = read_gitfile_gently(path);
	if (tmp) {
		path = tmp;
		if (stat(path, &st))
			die("Unable to stat submodule gitdir %s: %s (%d)",
			    path, strerror(errno), errno);
	}

	if (!S_ISDIR(st.st_mode))
		die("Submodule gitdir %s is not a directory", path);

	if (add_alt_odb(mkpath("%s/objects", path)))
		die("submodule odb %s could not be added as an alternate",
		    path);

	return 0;
}

static int check_gitlink(struct archiver_args *args, const unsigned char *sha1,
			 const char *path)
{
	switch (args->submodules) {
	case 0:
		return 0;

	case SUBMODULES_ALL:
		/* When all submodules are requested, we try to add any
		 * checked out submodules as alternate odbs. But we don't
		 * really care whether any particular submodule is checked
		 * out or not, we are going to try to traverse it anyways.
		 */
		include_repository(mkpath("%s.git", path));
		return READ_TREE_RECURSIVE;

	case SUBMODULES_CHECKEDOUT:
		/* If a repo is checked out at the gitlink path, we want to
		 * traverse into the submodule. But we ignore the current
		 * HEAD of the checked out submodule and always uses the SHA1
		 * recorded in the gitlink entry since we want the content
		 * of the archive to match the content of the <tree-ish>
		 * specified on the command line.
		 */
		if (!include_repository(mkpath("%s.git", path)))
			return READ_TREE_RECURSIVE;
		else
			return 0;

	default:
		die("archive.c: invalid value for args->submodules: %d",
		    args->submodules);
	}
}

struct archiver_context {
	struct archiver_args *args;
	write_archive_entry_fn_t write_entry;
};

static int write_archive_entry(const unsigned char *sha1, const char *base,
		int baselen, const char *filename, unsigned mode, int stage,
		void *context)
{
	static struct strbuf path = STRBUF_INIT;
	struct archiver_context *c = context;
	struct archiver_args *args = c->args;
	write_archive_entry_fn_t write_entry = c->write_entry;
	struct git_attr_check check[2];
	const char *path_without_prefix;
	int convert = 0;
	int err;
	enum object_type type;
	unsigned long size;
	void *buffer;

	strbuf_reset(&path);
	strbuf_grow(&path, PATH_MAX);
	strbuf_add(&path, base, baselen);
	strbuf_addstr(&path, filename);
	path_without_prefix = path.buf + args->baselen;

	setup_archive_check(check);
	if (!git_checkattr(path_without_prefix, ARRAY_SIZE(check), check)) {
		if (ATTR_TRUE(check[0].value))
			return 0;
		convert = ATTR_TRUE(check[1].value);
	}

	if (S_ISDIR(mode) || S_ISGITLINK(mode)) {
		strbuf_addch(&path, '/');
		if (args->verbose)
			fprintf(stderr, "%.*s\n", (int)path.len, path.buf);
		err = write_entry(args, sha1, path.buf, path.len, mode, NULL, 0);
		if (err)
			return err;
		return (S_ISDIR(mode) ? READ_TREE_RECURSIVE :
			check_gitlink(args, sha1, path.buf));
	}

	buffer = sha1_file_to_archive(path_without_prefix, sha1, mode,
			&type, &size, convert ? args->commit : NULL);
	if (!buffer)
		return error("cannot read %s", sha1_to_hex(sha1));
	if (args->verbose)
		fprintf(stderr, "%.*s\n", (int)path.len, path.buf);
	err = write_entry(args, sha1, path.buf, path.len, mode, buffer, size);
	free(buffer);
	return err;
}

int write_archive_entries(struct archiver_args *args,
		write_archive_entry_fn_t write_entry)
{
	struct archiver_context context;
	int err;

	if (args->baselen > 0 && args->base[args->baselen - 1] == '/') {
		size_t len = args->baselen;

		while (len > 1 && args->base[len - 2] == '/')
			len--;
		if (args->verbose)
			fprintf(stderr, "%.*s\n", (int)len, args->base);
		err = write_entry(args, args->tree->object.sha1, args->base,
				len, 040777, NULL, 0);
		if (err)
			return err;
	}

	context.args = args;
	context.write_entry = write_entry;

	err =  read_tree_recursive(args->tree, args->base, args->baselen, 0,
			args->pathspec, write_archive_entry, &context);
	if (err == READ_TREE_RECURSIVE)
		err = 0;
	return err;
}

static const struct archiver *lookup_archiver(const char *name)
{
	int i;

	if (!name)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(archivers); i++) {
		if (!strcmp(name, archivers[i].name))
			return &archivers[i];
	}
	return NULL;
}

static void parse_pathspec_arg(const char **pathspec,
		struct archiver_args *ar_args)
{
	ar_args->pathspec = get_pathspec(ar_args->base, pathspec);
}

static void parse_treeish_arg(const char **argv,
		struct archiver_args *ar_args, const char *prefix)
{
	const char *name = argv[0];
	const unsigned char *commit_sha1;
	time_t archive_time;
	struct tree *tree;
	const struct commit *commit;
	unsigned char sha1[20];

	if (get_sha1(name, sha1))
		die("Not a valid object name");

	commit = lookup_commit_reference_gently(sha1, 1);
	if (commit) {
		commit_sha1 = commit->object.sha1;
		archive_time = commit->date;
	} else {
		commit_sha1 = NULL;
		archive_time = time(NULL);
	}

	tree = parse_tree_indirect(sha1);
	if (tree == NULL)
		die("not a tree object");

	if (prefix) {
		unsigned char tree_sha1[20];
		unsigned int mode;
		int err;

		err = get_tree_entry(tree->object.sha1, prefix,
				     tree_sha1, &mode);
		if (err || !S_ISDIR(mode))
			die("current working directory is untracked");

		tree = parse_tree_indirect(tree_sha1);
	}
	ar_args->tree = tree;
	ar_args->commit_sha1 = commit_sha1;
	ar_args->commit = commit;
	ar_args->time = archive_time;
}

#define OPT__COMPR(s, v, h, p) \
	{ OPTION_SET_INT, (s), NULL, (v), NULL, (h), \
	  PARSE_OPT_NOARG | PARSE_OPT_NONEG, NULL, (p) }
#define OPT__COMPR_HIDDEN(s, v, p) \
	{ OPTION_SET_INT, (s), NULL, (v), NULL, "", \
	  PARSE_OPT_NOARG | PARSE_OPT_NONEG | PARSE_OPT_HIDDEN, NULL, (p) }

static int parse_archive_args(int argc, const char **argv,
		const struct archiver **ar, struct archiver_args *args)
{
	const char *format = "tar";
	const char *base = NULL;
	const char *remote = NULL;
	const char *exec = NULL;
	const char *output = NULL;
	const char *submodules = NULL;
	int compression_level = -1;
	int verbose = 0;
	int i;
	int list = 0;
	struct option opts[] = {
		OPT_GROUP(""),
		OPT_STRING(0, "format", &format, "fmt", "archive format"),
		OPT_STRING(0, "prefix", &base, "prefix",
			"prepend prefix to each pathname in the archive"),
		OPT_STRING(0, "output", &output, "file",
			"write the archive to this file"),
		{OPTION_STRING, 0, "submodules", &submodules, "kind",
			"include submodule content in the archive",
			PARSE_OPT_OPTARG, NULL, (intptr_t)"checkedout"},
		OPT__VERBOSE(&verbose),
		OPT__COMPR('0', &compression_level, "store only", 0),
		OPT__COMPR('1', &compression_level, "compress faster", 1),
		OPT__COMPR_HIDDEN('2', &compression_level, 2),
		OPT__COMPR_HIDDEN('3', &compression_level, 3),
		OPT__COMPR_HIDDEN('4', &compression_level, 4),
		OPT__COMPR_HIDDEN('5', &compression_level, 5),
		OPT__COMPR_HIDDEN('6', &compression_level, 6),
		OPT__COMPR_HIDDEN('7', &compression_level, 7),
		OPT__COMPR_HIDDEN('8', &compression_level, 8),
		OPT__COMPR('9', &compression_level, "compress better", 9),
		OPT_GROUP(""),
		OPT_BOOLEAN('l', "list", &list,
			"list supported archive formats"),
		OPT_GROUP(""),
		OPT_STRING(0, "remote", &remote, "repo",
			"retrieve the archive from remote repository <repo>"),
		OPT_STRING(0, "exec", &exec, "cmd",
			"path to the remote git-upload-archive command"),
		OPT_END()
	};

	argc = parse_options(argc, argv, opts, archive_usage, 0);

	if (remote)
		die("Unexpected option --remote");
	if (exec)
		die("Option --exec can only be used together with --remote");
	if (output)
		die("Unexpected option --output");

	if (!base)
		base = "";

	if (list) {
		for (i = 0; i < ARRAY_SIZE(archivers); i++)
			printf("%s\n", archivers[i].name);
		exit(0);
	}

	/* We need at least one parameter -- tree-ish */
	if (argc < 1)
		usage_with_options(archive_usage, opts);
	*ar = lookup_archiver(format);
	if (!*ar)
		die("Unknown archive format '%s'", format);

	args->compression_level = Z_DEFAULT_COMPRESSION;
	if (compression_level != -1) {
		if ((*ar)->flags & USES_ZLIB_COMPRESSION)
			args->compression_level = compression_level;
		else {
			die("Argument not supported for format '%s': -%d",
					format, compression_level);
		}
	}

	if (!submodules)
		args->submodules = 0;
	else if (!strcmp(submodules, "checkedout"))
		args->submodules = SUBMODULES_CHECKEDOUT;
	else if (!strcmp(submodules, "all"))
		args->submodules = SUBMODULES_ALL;
	else
		die("Invalid submodule kind: %s", submodules);
	args->verbose = verbose;
	args->base = base;
	args->baselen = strlen(base);

	return argc;
}

int write_archive(int argc, const char **argv, const char *prefix,
		int setup_prefix)
{
	const struct archiver *ar = NULL;
	struct archiver_args args;

	argc = parse_archive_args(argc, argv, &ar, &args);
	if (setup_prefix && prefix == NULL)
		prefix = setup_git_directory();

	parse_treeish_arg(argv, &args, prefix);
	parse_pathspec_arg(argv + 1, &args);

	git_config(git_default_config, NULL);

	return ar->write_archive(&args);
}
