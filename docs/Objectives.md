# SEC-LSM-MANAGER

The main objective of this project is to remake the Tyzen security-manager for AGL. The security context has evolved. In order to clearly define our need, I will start by reviewing the key points of the old security-manager.

- Defining SMACK access rights on files
- Define cynara rights
- Manage users
- A deamon that runs in the background and a library to interface with it
- Stores information in a database

The objectives of our new security manager will be :

- ✅ Be designed to accept several MACs : SMACK, SELinux; or not at all (token)
- ✅ Being atomic (either set up everything or nothing)
- ✅ To be activated by the socket
- ✅ To have a simulation mode
- ✅ Define cynagora rights effectively or to be requested at a later date
- ✅ Define additional rules based on permissions
- ✅ Without database

