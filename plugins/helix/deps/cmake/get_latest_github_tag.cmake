function(get_github_latest_release_tag owner repo out_var)
  set(_url "https://api.github.com/repos/${owner}/${repo}/releases/latest")
  set(_tmp "${CMAKE_BINARY_DIR}/latest_${repo}.json")
  file(DOWNLOAD ${_url} ${_tmp} STATUS stat)
  list(GET stat 0 code)
  if(NOT code EQUAL 0)
    message(FATAL_ERROR "${repo} download failed: ${stat}")
  endif()
  file(READ ${_tmp} _json)
  string(REGEX MATCH "\"tag_name\": ?\"([a-zA-Z0-9._-]+)\"" _match ${_json})
  string(REGEX REPLACE ".*\"tag_name\": ?\"([a-zA-Z0-9._-]+)\".*" "\\1" _tag
                       ${_match})
  set(${out_var}
      ${_tag}
      PARENT_SCOPE)
endfunction()