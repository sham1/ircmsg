project('ircmsg', 'c',
  default_options: ['c_std=c99'])

incdir = include_directories('include')

ircmsg_lib = library( 'ircmsg'
		    , 'src/parser.c'
                    , install: true
                    , include_directories: incdir
                    )

install_subdir('include/ircmsg', install_dir: get_option('includedir'))

ircmsg_dep = declare_dependency( link_with: ircmsg_lib
                               , include_directories: incdir
                               )

subdir('test')
