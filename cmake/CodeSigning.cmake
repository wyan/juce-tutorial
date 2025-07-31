# CodeSigning.cmake - Code signing configuration for Apple platforms

function(setup_code_signing)
    if(APPLE)
        # macOS/iOS code signing configuration
        set(CODESIGN_IDENTITY "Developer ID Application" CACHE STRING "Code signing identity")
        set(DEVELOPMENT_TEAM "" CACHE STRING "Apple Developer Team ID")
        set(PROVISIONING_PROFILE_SPECIFIER "" CACHE STRING "Provisioning profile for iOS")

        # Find available code signing identities
        execute_process(
            COMMAND security find-identity -v -p codesigning
            OUTPUT_VARIABLE AVAILABLE_IDENTITIES
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(AVAILABLE_IDENTITIES MATCHES "Developer ID Application")
            message(STATUS "Found code signing identity: Developer ID Application")

            # Apply to all targets globally
            set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODESIGN_IDENTITY}")

            if(DEVELOPMENT_TEAM)
                set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM}")
                message(STATUS "Using development team: ${DEVELOPMENT_TEAM}")
            endif()

            # iOS-specific provisioning
            if(PLATFORM_IOS AND PROVISIONING_PROFILE_SPECIFIER)
                set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "${PROVISIONING_PROFILE_SPECIFIER}")
                message(STATUS "Using provisioning profile: ${PROVISIONING_PROFILE_SPECIFIER}")
            endif()

            # Enable hardened runtime for notarization (macOS)
            if(PLATFORM_MACOS)
                set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
                set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--timestamp --options=runtime")
                message(STATUS "Enabled hardened runtime for notarization")
            endif()

            # iOS-specific settings
            if(PLATFORM_IOS)
                set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")
                set_property(GLOBAL PROPERTY XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0")
                message(STATUS "Configured iOS code signing")
            endif()

        else()
            message(WARNING "No suitable code signing identity found")
            message(STATUS "Available identities:")
            message(STATUS "${AVAILABLE_IDENTITIES}")
        endif()

    elseif(WIN32)
        # Windows code signing configuration
        set(SIGNTOOL_PATH "" CACHE PATH "Path to signtool.exe")
        set(CERTIFICATE_PATH "" CACHE PATH "Path to code signing certificate")
        set(CERTIFICATE_PASSWORD "" CACHE STRING "Certificate password")
        set(TIMESTAMP_URL "http://timestamp.digicert.com" CACHE STRING "Timestamp server URL")

        if(SIGNTOOL_PATH AND CERTIFICATE_PATH)
            message(STATUS "Windows code signing configured")
            message(STATUS "SignTool: ${SIGNTOOL_PATH}")
            message(STATUS "Certificate: ${CERTIFICATE_PATH}")
        else()
            message(STATUS "Windows code signing not configured (optional)")
        endif()
    endif()
endfunction()

# Function to sign a specific target
function(sign_target target_name)
    if(APPLE AND ENABLE_CODESIGNING)
        # macOS/iOS signing handled by Xcode build system
        set_target_properties(${target_name} PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODESIGN_IDENTITY}"
        )

        # Add notarization step for macOS
        if(PLATFORM_MACOS)
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Preparing ${target_name} for notarization..."
                COMMAND codesign --verify --verbose "$<TARGET_BUNDLE_DIR:${target_name}>"
                COMMENT "Verifying code signature for ${target_name}"
                VERBATIM
            )

            # Create notarization script
            configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/cmake/notarize.sh.in"
                "${CMAKE_CURRENT_BINARY_DIR}/notarize_${target_name}.sh"
                @ONLY
            )
        endif()

    elseif(WIN32 AND SIGNTOOL_PATH AND CERTIFICATE_PATH)
        # Windows signing
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND "${SIGNTOOL_PATH}" sign
                /f "${CERTIFICATE_PATH}"
                $<$<BOOL:${CERTIFICATE_PASSWORD}>:/p "${CERTIFICATE_PASSWORD}">
                /t "${TIMESTAMP_URL}"
                /fd SHA256
                /v
                "$<TARGET_FILE:${target_name}>"
            COMMENT "Code signing ${target_name}"
            VERBATIM
        )
    endif()
endfunction()

# Function to create distribution-ready packages
function(create_distribution_package target_name)
    if(APPLE)
        # Create DMG for macOS or IPA for iOS
        if(PLATFORM_MACOS)
            add_custom_target(${target_name}_dmg
                COMMAND ${CMAKE_COMMAND} -E echo "Creating DMG for ${target_name}..."
                COMMAND hdiutil create -volname "${target_name}"
                    -srcfolder "$<TARGET_BUNDLE_DIR:${target_name}>"
                    -ov -format UDZO
                    "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.dmg"
                DEPENDS ${target_name}
                COMMENT "Creating DMG package for ${target_name}"
                VERBATIM
            )
        elseif(PLATFORM_IOS)
            add_custom_target(${target_name}_ipa
                COMMAND ${CMAKE_COMMAND} -E echo "Creating IPA for ${target_name}..."
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/Payload"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "$<TARGET_BUNDLE_DIR:${target_name}>"
                    "${CMAKE_CURRENT_BINARY_DIR}/Payload/${target_name}.app"
                COMMAND cd "${CMAKE_CURRENT_BINARY_DIR}" &&
                    zip -r "${target_name}.ipa" Payload/
                COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/Payload"
                DEPENDS ${target_name}
                COMMENT "Creating IPA package for ${target_name}"
                VERBATIM
            )
        endif()

    elseif(WIN32)
        # Create ZIP package for Windows
        add_custom_target(${target_name}_zip
            COMMAND ${CMAKE_COMMAND} -E echo "Creating ZIP for ${target_name}..."
            COMMAND ${CMAKE_COMMAND} -E tar "cf" "${target_name}.zip" --format=zip
                "$<TARGET_FILE:${target_name}>"
            DEPENDS ${target_name}
            COMMENT "Creating ZIP package for ${target_name}"
            VERBATIM
        )
    endif()
endfunction()

# Validation function
function(validate_code_signing)
    if(APPLE AND ENABLE_CODESIGNING)
        # Check if required tools are available
        find_program(CODESIGN_TOOL codesign)
        find_program(SECURITY_TOOL security)

        if(NOT CODESIGN_TOOL)
            message(FATAL_ERROR "codesign tool not found. Install Xcode command line tools.")
        endif()

        if(NOT SECURITY_TOOL)
            message(FATAL_ERROR "security tool not found. Install Xcode command line tools.")
        endif()

        # Verify development team if specified
        if(DEVELOPMENT_TEAM)
            execute_process(
                COMMAND security find-identity -v -p codesigning
                OUTPUT_VARIABLE IDENTITY_OUTPUT
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            if(NOT IDENTITY_OUTPUT MATCHES "${DEVELOPMENT_TEAM}")
                message(WARNING "Development team ${DEVELOPMENT_TEAM} not found in available identities")
            endif()
        endif()

        message(STATUS "Code signing validation passed")

    elseif(WIN32 AND SIGNTOOL_PATH)
        # Verify SignTool exists
        if(NOT EXISTS "${SIGNTOOL_PATH}")
            message(FATAL_ERROR "SignTool not found at: ${SIGNTOOL_PATH}")
        endif()

        # Verify certificate exists
        if(CERTIFICATE_PATH AND NOT EXISTS "${CERTIFICATE_PATH}")
            message(FATAL_ERROR "Certificate not found at: ${CERTIFICATE_PATH}")
        endif()

        message(STATUS "Windows code signing validation passed")
    endif()
endfunction()
