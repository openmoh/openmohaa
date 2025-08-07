# Convert a shader file to a compilable C file

# INPUT_FILE, OUTPUT_FILE, SHADER_NAME must be set via -D

file(READ ${INPUT_FILE} CONTENTS)

string(REPLACE "\\" "\\\\" CONTENTS "${CONTENTS}") # Escape backslashes
string(REPLACE "\"" "\\\"" CONTENTS "${CONTENTS}") # Escape double quotes
string(REPLACE "\n" "\\n\"\n\"" CONTENTS "${CONTENTS}") # Escape newlines

set(OUTPUT_CONTENT "const char *fallbackShader_${SHADER_NAME} =\n\"${CONTENTS}\";\n")

file(WRITE ${OUTPUT_FILE} "${OUTPUT_CONTENT}")
