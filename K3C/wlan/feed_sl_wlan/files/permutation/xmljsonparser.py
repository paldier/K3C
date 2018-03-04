import xml.etree.ElementTree as ET
from collections import defaultdict
import json
import re
import sys, getopt
import copy

inputFile = ""
outFolder = ""

try:
	opts, args = getopt.getopt(sys.argv[1:],"hi:o:",["ifile=","ofolder="])
except getopt.GetoptError:
	print('xmljsonparser.py -i <inputfile> -o <outputfolder>')
	sys.exit(2)
for opt, arg in opts:
	if opt == '-h':
		print('xmljsonparser.py -i <inputfile> -o <outputfolder>')
		sys.exit()
	elif opt in ("-i", "--ifile"):
		inputFile = arg
	elif opt in ("-o", "--ofolder"):
		outFolder = arg

if inputFile == "":
	print("No input file given.")
	sys.exit()

def Tree():
    return defaultdict(Tree)
tree = ET.parse(inputFile)
root = tree.getroot()
finalJSON = {}
# XML contains sets with conditions and actions
for sets in root:
    conditions = []
    index = 0
    jsonkeys = []
    actions = []
    # check if set has a notify attribute
    if (len(sets.attrib) > 0):
        actionItem = {}
        if 'action' in sets.attrib:
            actionItem['action'] = sets.attrib['action']
        if 'message' in sets.attrib:
            actionItem['message'] = sets.attrib['message']
        actions.append(actionItem)
    for child in sets:
        if (len(child.attrib) == 0): # it is a condition
            items = child.text.split(",")
            ctag = child.tag.translate ({ord(c): "" for c in "{}"})
            for item in items:
                removeSpecialChars = item.translate ({ord(c): "" for c in "!@#$%^&()[]{};:<>?\|`~-=_+' "})
                if index == 0:
                    conditions.append(ctag+"-"+removeSpecialChars)
                else:
                    for i in range(len(conditions)):    
                        jsonkeys.append(conditions[i]+"-"+ctag+"-"+removeSpecialChars)
            #replace conditions with jsonkeys
            conlen = len(conditions)
            if (index > 0):
                for i in range(conlen):
                    conditions[i] = jsonkeys[i]
                for i in range(conlen,len(jsonkeys)):
                    conditions.append(jsonkeys[i])                
                jsonkeys = []
        else:
            #handle actions here
            actionItem = {}
            if 'action' in child.attrib:
                actionItem['action'] = child.attrib['action']
            if 'select' in child.attrib:
                actionItem['select'] = child.attrib['select']
            actionItem['value'] = child.text
            obj_params = child.tag.split("}")
            actionItem['object'] = obj_params[0][1:-1]
            actionItem['param'] = obj_params[1]
            actions.append(actionItem)            
        index = index + 1
    #print(conditions)
    #print(actions)
    for cond in conditions:
        if cond in finalJSON:
            finalJSON[cond] = finalJSON[cond] + actions
        else:
            finalJSON[cond] = actions
            
with open(outFolder+'/rules.json', 'w') as fp:
    json.dump(finalJSON, fp)