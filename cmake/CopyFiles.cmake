macro(configure_files src_dir dst_dir extension)
    message(STATUS "Configuring directory ${dst_dir}")
    make_directory(${dst_dir})

    file(GLOB files_to_copy RELATIVE ${src_dir} ${src_dir}/*.${extension})
    foreach(file_to_copy ${files_to_copy})
        set(file_path_to_copy ${src_dir}/${file_to_copy})
        if(NOT IS_DIRECTORY ${file_path_to_copy})
            message(STATUS "Configuring file ${file_to_copy}")
            configure_file(
                    ${file_path_to_copy}
                    ${dst_dir}/${file_to_copy}
                    @ONLY)
        endif(NOT IS_DIRECTORY ${file_path_to_copy})
    endforeach(file_to_copy)
endmacro(configure_files)