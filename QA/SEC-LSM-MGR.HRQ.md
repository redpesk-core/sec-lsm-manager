# High level requirements of redpesk-core/sec-lsm-manager

.VERSION: DRAFT

.AUTHOR: José Bollo [IoT.bzh]

.AUDIENCE: ENGINEERING

.DIFFUSION: CONFIDENTIAL

.REVIEW: IREV1

.git-id($Id$)

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER.

## Overview

Here is a short introduction. For a more detailed overview
see the document @SEC-LSM-MGR.OVE.

The SEC-LSM-MANAGER is a server that allows its client, the application
manager, to install and remove the security policy of applications.

![Figure: use of SEC-LSM-MANAGER](assets/SEC-LSM-MGR.OVE.fig-2.svg)

The SEC-LSM-MANAGER masters security policy by leveraging a Linux
kernel's LSM (Smack or SELinux) and by managing the permission database
cynagora.

![Figure: the component SEC-LSM-MANAGER](assets/SEC-LSM-MGR.OVE.fig-1.svg)

## Requirements

SEC-LSM-MANAGER is a service which helps installing and uninstalling
applications. For each client, it holds a context that records the
properties already set.

### System service

.REQUIREMENT SEC-LSM-MGR.HRQ-R-SYS-SER

SEC-LSM-MANAGER shall start automatically when its protected activation
socket receives a connection. The process initiating the connection is
named *client* in this document. The SEC-LSM-MANAGER shall serve the
client, and after a period of inactivity it shall stop.

**MOTIVATION**:
The SEC-LSM-MANAGER should not be invoked in command line but
should be integrated in the system configuration for running as service.

### Protected socket

.REQUIREMENT SEC-LSM-MGR.HRQ-R-PRO-SOC

SEC-LSM-MANAGER connection socket shall be protected in order to
allow connection only from authorized clients. The minimal
requirement is that the socket must be accessible only to processes
of the group `sec-lsm-manager`.

**IREV1:** "authorities" would mean some high-privileged process; why a
privileged process would want to connect to SEC-LSM-MANAGER? This seems
to be about clients which have the permission to use SEC-LSM-MANAGER, in
other words "authorized clients".

**MOTIVATION**
SEC-LSM-MANAGER is a sensitive service. It shall be accessible only
by authorized clients.

### Conformance to the protocol

.REQUIREMENT SEC-LSM-MGR.HRQ-R-CON-PRO

The server SEC-LSM-MANAGER shall implement the protocol described
in @SEC-LSM-MGR.PRO and shall strictly check that its clients are
respecting that protocol.

**MOTIVATION**:
The client accesses the service offered by SEC-LSM-MANAGER through the
correct use of the defined protocol.

### Installing security policy

.REQUIREMENT SEC-LSM-MGR.HRQ-R-INS-SEC-POL

The SEC-LSM-MANAGER shall, on client request, apply the security policy
corresponding to the current client context. On success, the client
context shall be cleared. On failure an error report is sent to the
client and the context is not cleared.

**MOTIVATION**:
Installing security policy during application setup is the main function
of SEC-LSM-MANAGER. The client shall first set all the properties in its
context and then apply it for installation.

Installation of security policy shall be done on existing files and
directories. Installation of security policy has the effect to change
the access rules of installed files or directories; hence it should be
done at the end of the installation process.

### Atomicity of installation

.REQUIREMENT SEC-LSM-MGR.HRQ-R-ATO-INS

When an installation request fails, SEC-LSM-MANAGER shall revert any
effective change it made in the context of that request.

**MOTIVATION**:
The system shall not be changed if, for some reason, the installation failed.

### Removing security policy

.REQUIREMENT SEC-LSM-MGR.HRQ-R-REM-SEC-POL

The SEC-LSM-MANAGER shall, on client request, remove the security policy
corresponding to the current client context. On success, the client
context shall be cleared. On failure an error report is sent to the
client and the context is not cleared.

**MOTIVATION**:

Removing security policy during uninstallation of applications is needed
for cleaning the database of permissions and access rights. Some
security policy can also forbid the removal of files by the installer;
hence requesting removal of security policy shall be done before the
real removal of files.

### Atomicity of removal

.REQUIREMENT SEC-LSM-MGR.HRQ-R-ATO-REM

When an uninstallation request fails, SEC-LSM-MANAGER shall revert any
effective change it made in the context of that request.

**MOTIVATION**:
The system shall not be changed if for some reason the removal failed.
