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

join_test_c = custom_target( 'join_test.c'
			   , output: 'join_test.c'
			   , input: [ 'join_test.py'
				    , 'yaml/msg-join.yaml'
				    , 'join_test.c.in'
				    ]
			   , command: [ prog_python
				      , '@INPUT@'
				      , '@OUTPUT@'
				      ]
			   )

join_test_exec = executable( 'join_test_exec'
			   , join_test_c
			   , dependencies: [ ircmsg_dep
					   , cmocka_dep
					   , ircmsg_test_dep
					   ]
			   )

test('join compliant', join_test_exec)
