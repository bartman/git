test_expect_success 'git apply scan' '
	git apply \
		"$TEST_DIRECTORY/t4110/patch1.patch" \
		"$TEST_DIRECTORY/t4110/patch2.patch" \
		"$TEST_DIRECTORY/t4110/patch3.patch" \
		"$TEST_DIRECTORY/t4110/patch4.patch" \
		"$TEST_DIRECTORY/t4110/patch5.patch" &&
	test_cmp new.txt "$TEST_DIRECTORY/t4110/expect"
'