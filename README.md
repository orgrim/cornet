Cornet
======

About
-----

The goal is to create a cron daemon which can share some cron jobs with other machines on a network. The following possibilities seems mandatory for a start:
* synchronize some cronjobs with other hosts
* allow to edit the crontab from any host
* be transparent: use compatible crontabs with other cron daemons, run aside with other cron daemons.

Source code
-----------

The source code, licensed under GNU GPL v2, is available on Github in https://github.com/orgrim/cornet

Features
--------

Mandatory for a proof of concept:

* shared crontab
* registrations/dicovery between nodes
* crontab synchronization
* edit with locks

For a real world use:

* SSL / IPv6
* cron stuff in a module
* private and public crontabs privée: allowing to replace the default cron daemon
* groups of nodes with shared crontab
* at, anacron

Docs:

* HTTP 1.1 (RFC 2616): http://www.faqs.org/rfcs/rfc2616.html
* Condition variables: http://www.cs.cf.ac.uk/Dave/C/node31.html
* Uniform Resource Identifier (RFC 2396): http://www.faqs.org/rfcs/rfc2396.html
* Path computing: "A Star Algorithm":http://en.wikipedia.org/wiki/A_Star_Search_Algorithm

TODO
----

* Move the following TODO stuff to the tracker
* threaded TCP server --- **done**
* threaded TCP client
* IPv6 support
* HTTP in a generic library, based on the RFC --- **in progress**
* Exchange data between nodes (on the "chandail"). Use URLs. RTFM on REST
* Parsing of crontab files
* Follow the clock and handle special cases like daylight saving time switches.
* Execute commands and take care of the shell environment
* crontab edition (unix domain socket? setuid binary (like vixie-cron)?)


Friendly competition
====================

* vixie-cron: ftp://ftp.isc.org/isc/cron/
* fcron: http://fcron.free.fr/
* dcron: http://apollo.backplane.com/FreeSrc/
* bcron: http://untroubled.org/bcron/
* mcron: http://www.gnu.org/software/mcron/design.html -- GNU

