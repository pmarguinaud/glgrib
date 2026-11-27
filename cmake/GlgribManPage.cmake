# cmake -DPROGRAM=... -DPAGE=... -DPOD2MAN=... -DOUTPUT=... -P GlgribManPage.cmake
#
# Run PROGRAM --pod and convert its output to a man page. Options::parse
# returns false after --pod (as after --help), so glgrib exits with status 1:
# the exit status is ignored, an empty output is an error.

execute_process (
  COMMAND ${PROGRAM} --pod
  OUTPUT_FILE ${OUTPUT}.pod
)

file (SIZE ${OUTPUT}.pod size)
if (size EQUAL 0)
  message (FATAL_ERROR "${PROGRAM} --pod produced no output")
endif ()

execute_process (
  COMMAND ${POD2MAN} -r "" -n ${PAGE} -c "" ${OUTPUT}.pod ${OUTPUT}
  COMMAND_ERROR_IS_FATAL ANY
)

file (REMOVE ${OUTPUT}.pod)
