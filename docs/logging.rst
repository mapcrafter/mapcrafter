=======
Logging
=======

Mapcrafter has its own logging facility which is configurable with a global logging
configuration file as well as with the normal render configuration files. You can
configure Mapcrafter to log its output into a log file or a local syslog daemon.

You can find your global logging configuration file with the ``mapcrafter --find-resources``
command, but it's usually installed as ``/etc/mapcrafter/logging.conf`` or directly
available as ``mapcrafter/src/logging.conf`` (if Mapcrafter is not system/user-wide
installed).

Here is a very simple example of a logging configuration file:

.. code-block:: ini

    [log:output]
    type = output
    verbosity = INFO
    
    [log:file]
    type = file
    verbosity = INFO
    file = /var/log/mapcrafter.log
    
    [log:syslog]
    type = syslog
    verbosity = INFO

Every log section (prefixed with ``log:``) configures one log sink.
You can configure the logging facility with those log sections in the global logging
configuration file, but you can also use those log sections in the normal render
configuration files. The log sections in the normal render configuration files are used
after the ones in the global logging configuration file, so you can use them to overwrite
the global logging configuration.

The names of the log sections are not relevant because you specify the type of the log
sink with the ``type`` option. An exception of this are file log sinks. You should make
sure that you do not use the same section name for file log sinks multiple times because
they are used for internal representation.

Available options
=================

General options
---------------

The following options are relevant for all log sink types.

``type = <type>``

    **Required**
    
    This is the type of the log sink you want to configure. Available types are:
    
    ``output``
      This is the default log output Mapcrafter shows when you run it. It is always
      enabled by default.
    ``file``
      This sink writes all log output into a log file.
    ``syslog``
      This sink sends all log output to the local syslog daemon.

``verbosity = <verbosity>``

    **Default:** ``INFO``
    
    This is the verbosity of the log sink, i.e. the minimum log level a message must
    have to be handled by the log sink. Available log levels are (according to `RFC 5424 <https://tools.ietf.org/html/rfc5424>`_ ):
    
    * ``DEBUG``, ``INFO``, ``NOTICE``, ``WARNING``, ``ERROR``, ``FATAL``, ``ALERT``,
      ``EMERGENCY``

``log_progress = true|false``

    **Default:** ``true`` (except output log)
    
    This option specifies whether the log sink should log progress messages.
    It is disabled by default for the output log because it is already using the
    animated progress bar. If you enable the ``--batch`` mode, this is also enabled for
    the output log and the animated progress bar is not shown.

Output and file log sink options
--------------------------------

The following options are only relevant for the output and file log sinks.

``format = <format>``

    **Default:** ``%(date) [%(level)] [%(logger)] %(message)``
    
    This is the format log messages are formatted with. You can use the following
    placeholders to specify the format of the log messages:
    
    =============== =======
    Placeholder     Meaning
    =============== =======
    ``%(date)``     Current date formatted with date format.
    ``%(level)``    Log level of the logged message.
    ``%(logger)``   Logger used to log this message (usually ``default`` or ``progress``).
    ``%(file)``     Source file name where this message was logged.
    ``%(line)``     Source file line number where this message was logged.
    ``%(message)``  The actually logged message.
    =============== =======


``date_format = <dateformat>``

    **Default:** ``%Y-%m-%d %H:%M:%S``
    
    This is the format the ``%(date)`` field is formatted with. Internally the
    std::strftime function is used to format the date field, so have a look at its
    `documentation <http://en.cppreference.com/w/cpp/chrono/c/strftime>`_ for the
    available placeholders.

File log options
----------------

The following option is only relevant for the file log sink.

``file = <file>``

    **Required**
    
    This option specifies the file the file log sink should output the log messages
    to.
