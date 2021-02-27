add_subdirectory(NetworkCore)
add_subdirectory(Replication)

set_property(TARGET "NetworkCore" PROPERTY FOLDER "Networking")
set_property(TARGET "Replication" PROPERTY FOLDER "Networking")