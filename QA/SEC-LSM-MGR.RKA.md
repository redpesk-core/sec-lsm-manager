# Risk analysis of redpesk-core/sec-lsm-manager

.VERSION: DRAFT

.AUTHOR: José Bollo [IoT.bzh]

.REVIEW: IREV1

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER.

The document @SEC-LSM-MGR.OVE describes SEC-LSM-MANAGER.

## Effect of signals

.RISK SEC-LSM-MGR.RKA-K-EFF-SIG
The process SEC-LSM-MGR is killed or crashes.

**Effect**:

- Clients of SEC-LSM-MGR are disconnect and receive error.
  Because the client receives a disconnection error, it can take the
  correct decision.

- Operations in progress are aborted. In that case, the system could be
  in an unexpected state because the action started by SEC-LSM-MGR is
  only partially completed.

**Mitigation**:

- The client must take correct action when SEC-LSM-MGR disconnects
  abruptly.

- The SEC-LSM-MGR should be able to rollback from an aborted transaction.
  (TODO)

## Usurpation of Unix Domain Socket

.RISK SEC-LSM-MGR.RKA-K-USU-UDS
The socket interface of SEC-LSM-MGR is usurped.

**Effect**: An application could impersonate the SEC-LSM-MGR.

**Mitigation**:

- The socket interface must be protected by DAC and MAC to ensure that only
the SEC-LSM-MGR can create the socket.

- Clients of SEC-LSM-MGR should check the identity of the service they
access, at least its UID and GID.

## Change binary

.RISK SEC-LSM-MGR.RKA-K-CHA-BIN
Binary of the SEC-LSM-MGR is changed.

**Effect**: An application could impersonate the SEC-LSM-MGR.

**Mitigation**:

- The binary must be protected by DAC and MAC to ensure that only
authorized process can replace SEC-LSM-MGR.

## Change configuration

.RISK SEC-LSM-MGR.RKA-K-CHA-CON
A configuration file of the SEC-LSM-MGR is changed.

**Effect**: An application could modify SEC-LSM-MGR policy or behavior.

**Mitigation**:

- All configuration files must be protected by DAC and MAC to ensure
that only authorized processes can modify it.

## Unauthorized client

.RISK SEC-LSM-MGR.RKA-K-UNA-CLI An unauthorized client spoofs an
authorized one and uses SEC-LSM-MGR. Example: the unix tool netcat can
easily be used to interact with SEC-LSM-MANAGER using the text protocol.

**Effect**: Features of SEC-LSM-MGR are exploited to alter security policy
of the system.

**Mitigation**:

- SEC-LSM-MGR must check using DAC, MAC and CYNAGORA if the client is
authorized. This check must be strong and cannot be tampered. See
@SEC-LSM-MGR.HRQ-R-PRO-SOC.

## Denial-of-service

.RISK SEC-LSM-MGR.RKA-K-DEN-SER
An unauthorized client connects repeatedly to SEC-LSM-MGR in order to
create a denial-of-service attack.

**Effect**: The service of SEC-LSM-MGR is not available to authorized
clients.

**Mitigation**:

- Using DAC and MAC, access to the socket must be possible only by authorized
clients.
