static int diff_use_color_default;
	if (pp->pattern)
		free(pp->pattern);
	return git_default_config(var, value);
static void print_line_count(int count)
		printf("0,0");
		printf("1");
		printf("1,%d", count);
static void copy_file_with_prefix(int prefix, const char *data, int size,
			fputs(set, stdout);
			putchar(prefix);
			fputs(reset, stdout);
		putchar(ch);
		printf("%s\n\\ No newline at end of file\n", reset);
	printf("%s--- %s%s%s\n%s+++ %s%s%s\n%s@@ -",
	       metainfo, a_name.buf, name_a_tab, reset,
	       metainfo, b_name.buf, name_b_tab, reset, fraginfo);
	print_line_count(lc_a);
	printf(" +");
	print_line_count(lc_b);
	printf(" @@%s\n", reset);
		copy_file_with_prefix('-', one->data, one->size, old, reset);
		copy_file_with_prefix('+', two->data, two->size, new, reset);
static void print_word(struct diff_words_buffer *buffer, int len, int color,
	fputs(diff_get_color(1, color), stdout);
	fwrite(ptr, len, 1, stdout);
	fputs(diff_get_color(1, DIFF_RESET), stdout);
			putchar('\n');
			putchar('\n');
			print_word(&diff_words->minus, len, DIFF_FILE_OLD, 1);
			print_word(&diff_words->plus, len, DIFF_FILE_NEW, 0);
			print_word(&diff_words->plus, len, DIFF_PLAIN, 0);
		putchar('\n');
		if (ecbdata->diff_words->minus.text.ptr)
			free (ecbdata->diff_words->minus.text.ptr);
		if (ecbdata->diff_words->plus.text.ptr)
			free (ecbdata->diff_words->plus.text.ptr);
static void emit_line(const char *set, const char *reset, const char *line, int len)
	fputs(set, stdout);
	fwrite(line, len, 1, stdout);
	fputs(reset, stdout);
		emit_line(set, reset, line, len);
		emit_line(set, reset, line, ecbdata->nparents);
		    stdout, set, reset, ws);
		printf("%s--- %s%s%s\n",
		       meta, ecbdata->label_path[0], reset, name_a_tab);
		printf("%s+++ %s%s%s\n",
		       meta, ecbdata->label_path[1], reset, name_b_tab);
		emit_line(diff_get_color(ecbdata->color_diff, DIFF_FRAGINFO),
			putchar('\n');
		emit_line(reset, reset, line, len);
		emit_line(plain, reset, line, len);
		emit_line(diff_get_color(ecbdata->color_diff, color),
static void show_name(const char *prefix, const char *name, int len,
	printf(" %s%s%-*s%s |", set, prefix, len, name, reset);
static void show_graph(char ch, int cnt, const char *set, const char *reset)
	printf("%s", set);
		putchar(ch);
	printf("%s", reset);
			show_name(prefix, name, len, reset, set);
			printf("  Bin ");
			printf("%s%d%s", del_c, deleted, reset);
			printf(" -> ");
			printf("%s%d%s", add_c, added, reset);
			printf(" bytes");
			printf("\n");
			show_name(prefix, name, len, reset, set);
			printf("  Unmerged\n");
		show_name(prefix, name, len, reset, set);
		printf("%5d ", added + deleted);
		show_graph('+', add, add_c, reset);
		show_graph('-', del, del_c, reset);
		putchar('\n');
	}
	printf("%s %d files changed, %d insertions(+), %d deletions(-)%s\n",
static void show_shortstats(struct diffstat_t* data)
	printf(" %d files changed, %d insertions(+), %d deletions(-)\n",
			printf("-\t-\t");
			printf("%d\t%d\t", file->added, file->deleted);
				write_name_quoted(file->name, stdout,
				fputs(file->print_name, stdout);
				putchar(options->line_termination);
				putchar('\0');
				write_name_quoted(file->from_name, stdout, '\0');
			write_name_quoted(file->name, stdout, '\0');
		printf("%s:%d: %s.\n", data->filename, data->lineno, err);
		emit_line(set, reset, line, 1);
		    stdout, set, reset, ws);
static void emit_binary_diff_body(mmfile_t *one, mmfile_t *two)
		printf("delta %lu\n", orig_size);
		printf("literal %lu\n", two->size);
		puts(line);
	printf("\n");
static void emit_binary_diff(mmfile_t *one, mmfile_t *two)
	printf("GIT binary patch\n");
	emit_binary_diff_body(one, two);
	emit_binary_diff_body(two, one);
			"[ 	]*([^;]*$\\)" },
	printf("%sdiff --git %s %s%s\n", set, a_one, b_two, reset);
		printf("%snew file mode %06o%s\n", set, two->mode, reset);
			printf("%s%s%s\n", set, xfrm_msg, reset);
		printf("%sdeleted file mode %06o%s\n", set, one->mode, reset);
			printf("%s%s%s\n", set, xfrm_msg, reset);
			printf("%sold mode %06o%s\n", set, one->mode, reset);
			printf("%snew mode %06o%s\n", set, two->mode, reset);
			printf("%s%s%s\n", set, xfrm_msg, reset);
			emit_binary_diff(&mf1, &mf2);
			printf("Binary files %s and %s differ\n",
			       lbl[0], lbl[1]);
		if (DIFF_OPT_TST(o, COLOR_DIFF_WORDS))
	data.ws_rule = whitespace_rule(data.filename);
	if ((lstat(name, &st) < 0) ||
	    !S_ISREG(st.st_mode) || /* careful! */
	    ce_match_stat(ce, &st, 0) ||
	    hashcmp(sha1, ce->sha1))
	/* we return 1 only when we can stat, it is a regular file,
	 * stat information matches, and sha1 recorded in the cache
	 * matches.  I.e. we know the file in the work tree really is
	 * the same as the <name, sha1> pair.
	return 1;
		if (convert_to_git(s->path, s->data, s->size, &buf)) {
		const char *cmd = external_diff_attr(name);
		printf("* Unmerged path %s\n", name);
		run_diff_cmd(pgm, p->one->path, NULL, NULL, NULL, NULL, o, 0);
	name  = p->one->path;
	other = (strcmp(name, p->two->path) ? p->two->path : NULL);
		run_diff_cmd(NULL, name, other, one, null, xfrm_msg, o, 0);
		run_diff_cmd(NULL, name, other, null, two, xfrm_msg, o, 0);
		run_diff_cmd(pgm, name, other, one, two, xfrm_msg, o,
			     complete_rewrite);
	builtin_checkdiff(name, other, p->one, p->two, o);
	if (diff_use_color_default)
	else
	if (!abbrev)
		return sha1_to_hex(sha1);
		printf(":%06o %06o %s ", p->one->mode, p->two->mode,
		       diff_unique_abbrev(p->one->sha1, opt->abbrev));
		printf("%s ", diff_unique_abbrev(p->two->sha1, opt->abbrev));
		printf("%c%03d%c", p->status, similarity_index(p),
			   inter_name_termination);
		printf("%c%c", p->status, inter_name_termination);
	if (p->status == DIFF_STATUS_COPIED || p->status == DIFF_STATUS_RENAMED) {
		write_name_quoted(p->one->path, stdout, inter_name_termination);
		write_name_quoted(p->two->path, stdout, line_termination);
		const char *path = p->one->mode ? p->one->path : p->two->path;
		write_name_quoted(path, stdout, line_termination);
	else if (fmt & DIFF_FORMAT_NAME)
		write_name_quoted(p->two->path, stdout, opt->line_termination);
static void show_file_mode_name(const char *newdelete, struct diff_filespec *fs)
		printf(" %s mode %06o ", newdelete, fs->mode);
		printf(" %s ", newdelete);
	write_name_quoted(fs->path, stdout, '\n');
static void show_mode_change(struct diff_filepair *p, int show_name)
		printf(" mode change %06o => %06o%c", p->one->mode, p->two->mode,
			write_name_quoted(p->two->path, stdout, '\n');
static void show_rename_copy(const char *renamecopy, struct diff_filepair *p)
	printf(" %s %s (%d%%)\n", renamecopy, names, similarity_index(p));
	show_mode_change(p, 0);
static void diff_summary(struct diff_filepair *p)
		show_file_mode_name("delete", p->one);
		show_file_mode_name("create", p->two);
		show_rename_copy("copy", p);
		show_rename_copy("rename", p);
			fputs(" rewrite ", stdout);
			write_name_quoted(p->two->path, stdout, ' ');
			printf("(%d%%)\n", similarity_index(p));
		show_mode_change(p, !p->score);
	if (output_format & (DIFF_FORMAT_DIFFSTAT|DIFF_FORMAT_SHORTSTAT|DIFF_FORMAT_NUMSTAT)) {
			show_shortstats(&diffstat);
			diff_summary(q->queue[i]);
				fputs(options->stat_sep, stdout);
				putchar(options->line_termination);
	if (S_ISGITLINK(one->mode)) {
		diff_fill_sha1_info(one);
		diff_fill_sha1_info(two);
		return !hashcmp(one->sha1, two->sha1);
	}