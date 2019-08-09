prog_python = import('python').find_installation('python3')

split_test_c = custom_target( 'split_test.c'
			    , output: 'split_test.c'
			    , input: [ 'split_test.py'
				     , 'yaml/msg-split.yaml'
				     , 'split_test.c.in'
				     ]
			    , command: [ prog_python
				       , '@INPUT@'
				       , '@OUTPUT@'
				       ]
			    )

split_test_exec = executable( 'split_test_exec'
			    , split_test_c
			    , dependencies: [ ircmsg_dep
					    , cmocka_dep
					    , ircmsg_test_dep
					    ]
			    )

test('split compliant', split_test_exec)
