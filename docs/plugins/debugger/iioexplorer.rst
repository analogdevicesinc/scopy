.. _iioexplorer:

IIO Explorer
================================================================================

This instrument represents the improved version of the original Debugger. The main
improvements are the ease of use and visualization of the data from the IIO context.

It is composed of 4 parts or views:

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/debugger/debugger-tool-v2.png
  :alt: Debugger version selection
  :align: center

#. Navigation View
#. Details View
#. Watch List View
#. Log Window

`The Navigation View` displays the context in the form of a tree view, making it easier to
visualize every device, channel and attribute that it offers. In addition to that, hovering
over any entry displays a tool tip with its type (Context, Device, Channel, etc.). At the
top of the view there is a filter bar, where the user can filter to find a specific entry.
The filtered navigation resets after the user clears the text from the filter bar.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/debugger/debugger-v2-nav.png
  :alt: Debugger V2 navigation
  :align: center

#. Navigation View
#. Filter Bar

`The Details View` is divided into 4 regions. The title region (1) contains the path of the
currently selected item from the Navigation View. It is composed of buttons and clicking
each one of them takes the user to that specific item. Next to it, there is the add/remove
from Watch List button that adds/removes the currently selected item to/from the
Watch List. In the right side there is a refresh button that, once
pressed, reads all the information displayed on the screen (the currently selected item and
its direct attributes and the attributes from the Watch List). The `Details View` (2) displays
the editable attributes of the currently selected item. By selecting the `IIO View`, a
read-only text view will appear that simulates the output from the `iio_info` program,
specific to the selected item.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/debugger/debugger-v2-details.png
  :alt: Debugger version selection
  :align: center

#. Title Region
#. Details View (GUI View + IIO View)
#. General Information

`The Watch List View` is table where the user can add the frequently used attributes for quick
access. This can be done by clicking the add button located near the title, in the `Details View`.
It is composed of 5 columns. The first is the `name` of the attribute (as displayed
in the Navigation View). The second one is the `value`, shown as an editable box or a
drop-down in case the user has a set of values from which they can choose, or in case the
user adds an item of type Context, Device or Channel, the value box will display `N/A` and
will not be editable. The 3rd column is the `Type` column that displays the type of the
added item (Channel, Channel Attribute, Device Attribute, etc.). The 4th column shows the
path of that item, starting from the context, all the way to the attribute. The last column
displays a button that deletes the entire row from the watchlist.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/debugger/debugger-v2-watchlist.png
  :alt: Debugger version selection
  :align: center

`The Log Window` can be accessed by clicking the `Log` button next to the `IIO Attributes`
button and will display a log with all the operations with their timestamp, whether
they were a read (R) or write (W) operation, the return code (`SUCCESS` means the return code
is 0), the path of the modified item and the value read (in case of a write operation, the
previous value is also displayed).

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/debugger/debugger-log-v2.png
  :alt: Debugger V2 log window
  :align: center
