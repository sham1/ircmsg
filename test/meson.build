cmocka_dep = dependency('cmocka')

ircmsg_test_lib = static_library( 'ircmsg_test_internal'
				, 'parser_test.c'
				, dependencies: [ ircmsg_dep
						]
				)

ircmsg_test_dep = declare_dependency(link_with: ircmsg_test_lib)

failure_exec = executable( 'parse_fail_test'
			 , 'parser_failure.c'
			 , dependencies: [ ircmsg_dep
					 , cmocka_dep
					 , ircmsg_test_dep
					 ]
			 )

success_exec = executable( 'parse_success_test'
			 , 'parser_success.c'
			 , dependencies: [ ircmsg_dep
					 , cmocka_dep
					 , ircmsg_test_dep
					 ]
			 )

test('parse failures', failure_exec)
test('parse successes', success_exec)
