==================
Hacking Mapcrafter
==================

This page is all about making changes beyond the normal configuration file
options. It assumes some knowledge of the internals of Mapcrafter,
Blockcrafer, C++, Javascript and HTML.

This page is a work-in-progress.

Web Output
==========

Mapcrafter renders to html, images and javascript, and uses `leaflet.js <https://leafletjs.com/>`_
to enable the interactive map. You can make changes to the ``template_dir``
files to customise these files or leaflet.js.


Template Variables
------------------

Mapcrafter replaces variables delimited by curly braces ``{}`` in the 
``global:template_dir`` *index.html* file  These are listed below for 
reference, although probably aren't that useful.

=====================  ==================================================  ====================
Variable               Description                                         Example
=====================  ==================================================  ====================
``{version}``          Mapcrafter version                                  2.4
``{lastUpdate}``       Datestamp of last update.                           01.10.2018, 21:12:44
``{backgroundColor}``  Hex background colour. See ``background_color``     #D0D0D0
=====================  ==================================================  ====================
