/* How to run this script:
 * scopy --script read_dmm.js
 */

print("Connecting to 192.168.2.1...")
if (!launcher.connect("ip:192.168.2.1")) {
	print("Failed!")
	/* TODO: Exit the script, if we cannot connect. */
} else {
	print("Connected!")
}

/* Use DC mode */
dmm.mode_ac_ch1 = false
dmm.mode_ac_ch2 = false

/* Start the capture */
dmm.running = true

/* TODO: we must sleep here - otherwise the values will always be garbage. */
print("Value channel 1: " + dmm.value_ch1)
print("Value channel 2: " + dmm.value_ch2)

/* Stop the capture and disconnect. */
dmm.running = false
launcher.disconnect()
