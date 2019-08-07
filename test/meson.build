cmocka_dep = dependency('cmocka')

failure_exec = executable( 'parse_fail_test'
			 , 'parser_failure.c'
			 , dependencies: [ ircmsg_dep
					 , cmocka_dep
					 ]
			 )

test('parse failures', failure_exec)
