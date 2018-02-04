set(GRAPHICS_VAL 0)

if (VERBOSE)
	set (VERBOSE_ON 1)
else()
	set (VERBOSE_ON 0)
endif()

if(V)
	set (VERIFICA_ON 1)
else()
	set (VERIFICA_ON 0)
endif()
if(P)
	message (Inserted binary location for graphics: ${GNUPLOT})
	file (MAKE_DIRECTORY "build/graphics")
	add_custom_target(graphviz
		dot -Tpng graphics/prove.gv -o graphics/prove.png
		)
	set (PROVE_ON 1)
else()
	set (PROVE_ON 0)
endif()

