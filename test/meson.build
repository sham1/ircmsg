cmocka_dep = dependency('cmocka')

ircmsg_test_lib = static_library( 'ircmsg_test_internal'
				, 'parser_test.c'
				, 'serializer_test.c'
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

serialize_len_exec = executable( 'serialize_length_test'
			       , 'serializer_length.c'
			       , dependencies: [ ircmsg_dep
					       , cmocka_dep
					       , ircmsg_test_dep
					       ]
			       )

serialize_basic_exec = executable( 'serialize_basic_test'
				 , 'serializer_basic.c'
				 , dependencies: [ ircmsg_dep
						 , cmocka_dep
						 , ircmsg_test_dep
						 ]
				 )

test('parse failures', failure_exec)
test('parse successes', success_exec)
test('serializer length', serialize_len_exec)
test('serializer basic', serialize_basic_exec)

subdir('compliance-tests')
