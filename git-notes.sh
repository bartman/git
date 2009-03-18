#!/bin/sh

USAGE="(edit | show) [commit]"
. git-sh-setup

test -n "$3" && usage

test -z "$1" && usage
ACTION="$1"; shift

test -z "$GIT_NOTES_REF" && GIT_NOTES_REF="$(git config core.notesref)"
test -z "$GIT_NOTES_REF" && GIT_NOTES_REF="refs/notes/commits"

COMMIT=$(git rev-parse --verify --default HEAD "$@") ||
die "Invalid commit: $@"

case "$ACTION" in
edit)
	if [ "${GIT_NOTES_REF#refs/notes/}" = "$GIT_NOTES_REF" ]; then
		die "Refusing to edit notes in $GIT_NOTES_REF (outside of refs/notes/)"
	fi

	MESSAGE="$GIT_DIR"/new-notes-$COMMIT
	trap '
		test -f "$MESSAGE" && rm "$MESSAGE"
	' 0

	GIT_NOTES_REF= git log -1 $COMMIT | sed "s/^/#/" > "$MESSAGE"

	GIT_INDEX_FILE="$MESSAGE".idx
	export GIT_INDEX_FILE

	CURRENT_HEAD=$(git show-ref "$GIT_NOTES_REF" | cut -f 1 -d ' ')
	if [ -z "$CURRENT_HEAD" ]; then
		PARENT=
	else
		PARENT="-p $CURRENT_HEAD"
		git read-tree "$GIT_NOTES_REF" || die "Could not read index"
		git cat-file blob :$COMMIT >> "$MESSAGE" 2> /dev/null
	fi

	core_editor="$(git config core.editor)"
	${GIT_EDITOR:-${core_editor:-${VISUAL:-${EDITOR:-vi}}}} "$MESSAGE"

	grep -v ^# < "$MESSAGE" | git stripspace > "$MESSAGE".processed
	mv "$MESSAGE".processed "$MESSAGE"
	if [ -s "$MESSAGE" ]; then
		BLOB=$(git hash-object -w "$MESSAGE") ||
			die "Could not write into object database"
		git update-index --add --cacheinfo 0644 $BLOB $COMMIT ||
			die "Could not write index"
	else
		test -z "$CURRENT_HEAD" &&
			die "Will not initialise with empty tree"
		git update-index --force-remove $COMMIT ||
			die "Could not update index"
	fi

	TREE=$(git write-tree) || die "Could not write tree"
	NEW_HEAD=$(echo Annotate $COMMIT | git commit-tree $TREE $PARENT) ||
		die "Could not annotate"
	git update-ref -m "Annotate $COMMIT" \
		"$GIT_NOTES_REF" $NEW_HEAD $CURRENT_HEAD
;;
show)
	git rev-parse -q --verify "$GIT_NOTES_REF":$COMMIT > /dev/null ||
		die "No note for commit $COMMIT."
	git show "$GIT_NOTES_REF":$COMMIT
;;
*)
	usage
esac
