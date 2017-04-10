project('spielpatz', 'cpp')

compiler = meson.get_compiler('cpp')

glfw  = dependency('glfw3')
glew  = dependency('glew')
gl    = dependency('gl')
glib  = dependency('glib-2.0')
cairo = dependency('cairo')

cpp_flags = ['-std=c++11', '-Wall']
ld_flags  = ['-lstdc++']

executable('spielpatz',
	   sources: [ 'main.cc' ],
	   cpp_args: cpp_flags,
	   link_args: ld_flags,
	   dependencies: [
	     glfw,
	     glew,
	     gl,
	     glib,
	     cairo])
