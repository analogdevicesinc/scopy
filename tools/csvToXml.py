from tkinter import filedialog as fd
import xml.etree.cElementTree as ET
import csv 
from pathlib import Path

  

#Find the csv file to convert
file = fd.askopenfilename(title = "Select file", filetypes = (("CSV Files","*.csv"),))

registers = []
with open(file) as file_obj: 
        
    reader_obj = csv.reader(file_obj)  
    
    #get the mane of the file and use it as device name       
    dev = Path(file).stem
    device = ET.Element(dev)

    # generate registers
    for row in reader_obj:

        # access to register can be R or W or R/W 
        access = ""
        if (row[8] == "1") :
            access += "R"
        if (row[9] == "1") :
            if (access == "R"):
                access += "/"
            access += "W" 
            
        register = ET.SubElement(device, "Register")
        name = ET.SubElement(register, "Name").text = row[0]
        address = ET.SubElement(register, "Address").text = hex(int(row[2]))
        desctiption = ET.SubElement(register, "Description")
        exists = ET.SubElement(register, "Exists").text = "True"
        width = ET.SubElement(register, "Width").text = row[5]
        notes = ET.SubElement(register, "Notes")

        bitFields = ET.SubElement(register, "BitFields")
        bitField = ET.SubElement(bitFields, "BitField")
        bitFieldName = ET.SubElement(bitField, "Name").text = row[0]
        bitFieldAccess = ET.SubElement(bitField, "Access").text = access
        bitFieldDefaultValue = ET.SubElement(bitField, "DefaultValue").text = row[12]
        bitFieldDescription = ET.SubElement(bitField, "Description")
        bitFieldWidth = ET.SubElement(bitField, "Width").text = row[5]
        bitFieldNotes = ET.SubElement(bitField, "Notes")
        bitFieldBitOffset = ET.SubElement(bitField, "BitOffset").text = "0"
        bitFieldRegOffset = ET.SubElement(bitField, "RegOffset").text = row[4]
        bitFieldSliceWidth = ET.SubElement(bitField, "SliceWidth").text = "1"
        
        
    # Write the XML data to a file 
    tree = ET.ElementTree(device)
    ET.indent(tree, space="\t", level=0)
    tree.write(dev + ".xml")    

