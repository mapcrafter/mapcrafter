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

As you can see, every log section (prefixed with ``log:``) configures one log sink.
The names of the sections are not relevant, you specify the type of the log sink with
the ``type`` options.

Available options
=================

``type = <type>``

    **Required**
    
    This is the type of the log sink you want to configure. Available types are:
    
    ``output``
      This is the default log output Mapcrafter shows when you run it. It is always
      enabled by default
    ``file``
      This sink writes all log output into a log file.
    ``syslog``
      This sink sends all log output to the local syslog daemon.

``verbosity = <verbosity>``

    **Default:** ``INFO``
    
    This is the verbosity of the log sink, i.e. the minimum log level of messages the log
    sink outputs messages. Available log levels are (according to syslog):
    
    * ``DEBUG``, ``INFO``, ``NOTICE``, ``WARNING``, ``ERROR``, ``FATAL``, ``ALERT``,
      ``EMERGENCY``

``log_progress = true|false``

    **Default:** ``true`` (except output log)
    
    This option specifies whether the log sink should log progress messages.
    It is disabled by default for the output log because it is already using the
    animated progress bar. If you enable the ``--batch`` mode, this is also enabled for
    the output log and the animated progress bar is not shown.

``format = <format>``

    **Default:** ``%(date) [%(level)] [%(logger)] %(message)``
    
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

``file = <file>``

    **Required if file log**
