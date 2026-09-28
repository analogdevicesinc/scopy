# ad9361-regmap-demo

Demo package showing how a device package can customize the Register Map plugin
(option A in `regmap_package_extension.md`, same pattern as `rfpowermeter`).

When a context containing `ad9361-phy` connects:

1. `RegmapPlugin` (priority 3) connects first and adds `ad9361-phy` with the default XML and device access.
2. `Ad9361RegmapPlugin` (priority 2, no tools) finds it with `m_device->getPluginByName("RegmapPlugin")` and calls
   - `setDeviceXml("ad9361-phy", <pkg>/resources/ad9361-regmap-demo/ad9361-phy-demo.xml)` (16 registers, descriptions prefixed with `[pkg]`)
   - `setDeviceAccess("ad9361-phy", FileRegisterReadStrategy, FileRegisterWriteStrategy)`
3. Register reads/writes now go to a values file instead of the device.

## Values file

- Template: `resources/ad9361-regmap-demo/ad9361-phy-values.csv` (`<address>,<value>` in hex, `#` comments).
- On first connect it is copied to `<scopy settings folder>/ad9361-regmap-demo/ad9361-phy-values.csv`, and that copy is used for reads and writes.
  The path is shown in the status bar and logged under `Ad9361RegmapPlugin`.
- The file is re-read on every access, so editing it while Scopy runs and pressing Read shows the new value.
- Delete the copy to reset it to the template values.

## Try it

1. Start the emulator with the pluto XML: `iio-emu generic packages/generic-plugins/emu-xml/pluto.xml`.
2. Connect to `ip:127.0.0.1` in Scopy and open Register map.
3. Select `ad9361-phy`. Only 16 `[pkg]` registers are shown, and Read returns the values from the CSV (e.g. `0x000` -> `0x5a`).
4. Write a value, then check that the CSV copy is updated.
5. Disable the `Ad9361RegmapPlugin` plugin (or remove the package) and reconnect. Regmap goes back to the full XML and device access.
