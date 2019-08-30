project('ircmsg', 'c',
	default_options: ['c_std=c99'],
	version: '1.0.1')

incdir = include_directories('include')

ircmsg_lib = library( 'ircmsg'
		    , 'src/parser.c'
		    , 'src/serializer.c'
                    , install: true
                    , include_directories: incdir
		    , version: '1.0.1'
                    )

install_subdir('include/ircmsg', install_dir: get_option('includedir'))

pkg = import('pkgconfig')
pkg.generate(ircmsg_lib)

ircmsg_dep = declare_dependency( link_with: ircmsg_lib
                               , include_directories: incdir
                               )

if get_option('tests')
  subdir('test')
endif
