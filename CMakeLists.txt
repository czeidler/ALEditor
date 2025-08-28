cmake_minimum_required(VERSION 2.6...4.1)
project(ALEditor)

include_directories(
	headers/charlemagne
	headers/components
	headers/customization
	headers/editor
	headers/haiku_private/alm
	headers/haiku_private/linearspec
	/boot/system/develop/headers/private/shared
	/boot/system/develop/headers/private/interface
)

add_library(ale SHARED
	src/charlemagne/CInterface.cpp
	src/charlemagne/PArgs.cpp
	src/charlemagne/PData.cpp
	src/charlemagne/PMethod.cpp
	src/charlemagne/PObject.cpp
	src/charlemagne/PObjectBroker.cpp
	src/charlemagne/PProperty.cpp
	src/charlemagne/PValue.cpp
	src/components/CustomTypes.cpp
	src/components/UIComponents.cpp
	src/customization/Actions.cpp
	src/customization/CustomizableRoster.cpp
	src/customization/CustomizableView.cpp
	src/customization/Object.cpp
	src/customization/Customizable.cpp
	src/customization/CustomizableNodeFactory.cpp
	src/customization/CustomizableRosterView.cpp
	src/customization/Mangle.cpp
	src/customization/TrashView.cpp
	src/editor/AreaRemoval.cpp
	src/editor/ALMEditor.cpp
	src/editor/LayoutEditView.cpp
	src/editor/EditAction.cpp
	src/editor/EditAnimation.cpp
	src/editor/InfoSystem.cpp
	src/editor/OverlapManager.cpp
	src/editor/EditActionAreaDragging.cpp
	src/editor/EditActionResizing.cpp
	src/editor/EditorWindow.cpp
	src/editor/LayoutArchive.cpp
)

target_link_libraries(ale shared)

add_executable(ALEditor ALEditor.cpp)
ADD_CUSTOM_COMMAND(TARGET ALEditor POST_BUILD
	COMMAND rc -o ALEditor.rsrc ${CMAKE_CURRENT_SOURCE_DIR}/res/ALEditor.rdef
	COMMENT "Compiling resources"
)
ADD_CUSTOM_COMMAND(TARGET ALEditor POST_BUILD
	COMMAND xres -o ALEditor ALEditor.rsrc
	COMMENT "Adding resources to executable"
)
ADD_CUSTOM_COMMAND(TARGET ALEditor POST_BUILD
	COMMAND mimeset --all ALEditor
	COMMENT "Adjusting MIME types"
)

target_link_libraries(ALEditor ${CORELIBS} be alm tracker ale)
