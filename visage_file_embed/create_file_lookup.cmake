if (NOT DEFINED FILE_LIST)
  message(FATAL_ERROR "Required FILE_LIST is not set")
endif ()

if (NOT DEFINED DEST_FILE)
  message(FATAL_ERROR "Required DEST_FILE is not set")
endif ()

if (NOT DEFINED INCLUDE_FILE)
  message(FATAL_ERROR "Required INCLUDE_FILE is not set")
endif ()

if (NOT DEFINED VAR_NAMESPACE)
  message(FATAL_ERROR "Required VAR_NAMESPACE is not set")
endif ()

function(get_var_name ORIGINAL_FILE)
endfunction()

file(WRITE "${DEST_FILE}" "// Generated file, do not edit\n")
file(APPEND "${DEST_FILE}" "#include \"${INCLUDE_FILE}\"\n")
file(APPEND "${DEST_FILE}" "namespace ${VAR_NAMESPACE} {\n")
file(APPEND "${DEST_FILE}" "  ::visage::EmbeddedFile getFileByName(const std::string& filename) {\n")

foreach (FILE ${FILE_LIST})
  string(REGEX MATCH "([^/]+)$" VAR_NAME ${FILE})
  string(REGEX REPLACE "\\.| |-" "_" VAR_NAME ${VAR_NAME})
  file(APPEND "${DEST_FILE}" "    if (filename == \"${VAR_NAME}\")\n")
  file(APPEND "${DEST_FILE}" "      return ${VAR_NAME};\n")
endforeach ()
file(APPEND "${DEST_FILE}" "    return { nullptr, 0 };\n")
file(APPEND "${DEST_FILE}" "  }\n")
file(APPEND "${DEST_FILE}" "}\n")
