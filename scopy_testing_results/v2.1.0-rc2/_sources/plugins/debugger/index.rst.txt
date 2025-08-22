.. _debugger_index:


Debugger
=====================================================================
The debugger plugin is used to examine IIO contexts and modify individual IIO attributes,
as well as examining the structure of an IIO context. The more important attributes are
also available in the other plugins, but this tool provides a full overview of the whole
context, giving the user the ability to read/modify every  individual attribute. This
makes it easier to configure any board in specific ways.

Overview
---------------------------------------------------------------------
This plugin is generic and can work with any valid context. It is composed of one
instrument that has 2 versions. The default version (version 2) is the IIO Explorer.
Users also have the ability to load version one of the debugger plugin by accessing the
`Preferences` panel on the left side and unchecking the `Use debugger V2 plugin` box.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/debugger/debugger-pref-version.png
  :alt: Debugger version selection
  :align: center

Contents
---------------------------------------------------------------------

The plugin contains 1 instrument that can be the Debugger (v1) or the IIO Explorer (v2):

.. toctree::
   :maxdepth: 3

   debugger
   iioexplorer
