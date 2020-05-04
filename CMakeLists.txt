art_make(LIB_LIBRARIES
            larcoreobj_SummaryData
            lardataobj_Simulation
            ${ART_FRAMEWORK_CORE}
            ${ART_FRAMEWORK_PRINCIPAL}
            ${ART_FRAMEWORK_SERVICES_REGISTRY}
            ${ART_FRAMEWORK_SERVICES_OPTIONAL}
            art_Persistency_Common canvas
            art_Persistency_Provenance canvas
            art_Utilities canvas
            cetlib cetlib_except
            ${MF_MESSAGELOGGER}
            ${MF_UTILITIES}
            ${FHICLCPP}
            ${ROOT_BASIC_LIB_LIST}
            ${DK2NU_TREE}
      MODULE_LIBRARIES
            larcoreobj_SummaryData
            lardataobj_Simulation
            ${ART_FRAMEWORK_CORE}
            ${ART_FRAMEWORK_PRINCIPAL}
            ${ART_FRAMEWORK_SERVICES_REGISTRY}
            ${ART_FRAMEWORK_SERVICES_OPTIONAL}
            art_Persistency_Common canvas
            art_Persistency_Provenance canvas
            art_Utilities canvas
            cetlib cetlib_except
            ${MF_MESSAGELOGGER}
            ${MF_UTILITIES}
            ${FHICLCPP}
            ${ROOT_BASIC_LIB_LIST}
            ${DK2NU_TREE}
            )

#add_subdirectory(job)

install_headers()
install_fhicl()
install_source()

