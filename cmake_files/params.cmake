if (DECAY)
	set (DECAY_VAL ${DECAY})
else()
	set (DECAY_VAL 1)
endif()
if (BONUS)
	set (BONUS_VAL ${BONUS})
else()
	set (BONUS_VAL ${DECAY_VAL})
endif()


