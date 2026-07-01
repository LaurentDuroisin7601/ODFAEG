
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was odfaegConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# Charger les targets générés automatiquement
include("${CMAKE_CURRENT_LIST_DIR}/odfaegTargets.cmake")

# Déterminer le préfixe d'installation (3 niveaux au-dessus)
get_filename_component(_ODFAEG_PREFIX "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
get_filename_component(_ODFAEG_PREFIX "${_ODFAEG_PREFIX}" DIRECTORY)
get_filename_component(_ODFAEG_PREFIX "${_ODFAEG_PREFIX}" DIRECTORY)

# Racines des modules et PCM
set(_ODFAEG_MODULE_ROOT "${_ODFAEG_PREFIX}/share/odfaeg/modules")
set(_ODFAEG_PCM_ROOT    "${_ODFAEG_PREFIX}/share/odfaeg/pcm")

# ------------------------------------------------------------------
# 1. Lier les bibliothèques normales à leurs modules
# ------------------------------------------------------------------

target_link_libraries(odfaeg::odfaeg-core     INTERFACE odfaeg::odfaeg-core-mod)
target_link_libraries(odfaeg::odfaeg-math     INTERFACE odfaeg::odfaeg-math-mod)
target_link_libraries(odfaeg::odfaeg-entity   INTERFACE odfaeg::odfaeg-entity-mod)
target_link_libraries(odfaeg::odfaeg-physics  INTERFACE odfaeg::odfaeg-physics-mod)
target_link_libraries(odfaeg::odfaeg-window   INTERFACE odfaeg::odfaeg-window-mod)
target_link_libraries(odfaeg::odfaeg-graphics INTERFACE odfaeg::odfaeg-graphics-mod)

# ------------------------------------------------------------------
# 2. Ajouter les chemins PCM pour Clang
# ------------------------------------------------------------------

target_compile_options(odfaeg::odfaeg-core-mod     INTERFACE -fprebuilt-module-path=${_ODFAEG_PCM_ROOT}/Core)
target_compile_options(odfaeg::odfaeg-math-mod     INTERFACE -fprebuilt-module-path=${_ODFAEG_PCM_ROOT}/Math)
target_compile_options(odfaeg::odfaeg-entity-mod   INTERFACE -fprebuilt-module-path=${_ODFAEG_PCM_ROOT}/Entity)
target_compile_options(odfaeg::odfaeg-physics-mod  INTERFACE -fprebuilt-module-path=${_ODFAEG_PCM_ROOT}/Physics)
target_compile_options(odfaeg::odfaeg-window-mod   INTERFACE -fprebuilt-module-path=${_ODFAEG_PCM_ROOT}/Window)
target_compile_options(odfaeg::odfaeg-graphics-mod INTERFACE -fprebuilt-module-path=${_ODFAEG_PCM_ROOT}/Graphics)

# ------------------------------------------------------------------
# 3. Dépendances inter-modules
# ------------------------------------------------------------------

target_link_libraries(odfaeg::odfaeg-math-mod     INTERFACE odfaeg::odfaeg-core-mod)
target_link_libraries(odfaeg::odfaeg-entity-mod   INTERFACE odfaeg::odfaeg-math-mod odfaeg::odfaeg-core-mod)
target_link_libraries(odfaeg::odfaeg-physics-mod  INTERFACE odfaeg::odfaeg-entity-mod odfaeg::odfaeg-math-mod odfaeg::odfaeg-core-mod)
target_link_libraries(odfaeg::odfaeg-window-mod   INTERFACE odfaeg::odfaeg-physics-mod odfaeg::odfaeg-entity-mod odfaeg::odfaeg-math-mod odfaeg::odfaeg-core-mod)
target_link_libraries(odfaeg::odfaeg-graphics-mod INTERFACE odfaeg::odfaeg-window-mod odfaeg::odfaeg-physics-mod odfaeg::odfaeg-entity-mod odfaeg::odfaeg-math-mod odfaeg::odfaeg-core-mod)

# ------------------------------------------------------------------
# 4. Inclure les dossiers de modules (utile pour CLion)
# ------------------------------------------------------------------

target_include_directories(odfaeg::odfaeg-core-mod     INTERFACE ${_ODFAEG_MODULE_ROOT}/Core)
target_include_directories(odfaeg::odfaeg-math-mod     INTERFACE ${_ODFAEG_MODULE_ROOT}/Math)
target_include_directories(odfaeg::odfaeg-entity-mod   INTERFACE ${_ODFAEG_MODULE_ROOT}/Entity)
target_include_directories(odfaeg::odfaeg-physics-mod  INTERFACE ${_ODFAEG_MODULE_ROOT}/Physics)
target_include_directories(odfaeg::odfaeg-window-mod   INTERFACE ${_ODFAEG_MODULE_ROOT}/Window)
target_include_directories(odfaeg::odfaeg-graphics-mod INTERFACE ${_ODFAEG_MODULE_ROOT}/Graphics)

# Fin
