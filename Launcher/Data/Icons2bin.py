from PIL import Image

icon_res_x = 32
icon_res_y = 32

im = Image.open("icons.png", "r")
pix_val = list(im.getdata())

channels = 1
totalsize = im.width * im.height * channels

with open("../src/icons.h", "w") as f:
  f.write("const int icons_length = %i;\n" % totalsize)
  f.write("const int icons_width = %i;\n" % im.width)
  f.write("const int icons_height = %i;\n" % im.height)
  f.write("const int icons_rows = %i;\n" % (im.height/icon_res_y))
  f.write("const int icons_cols = %i;\n" % (im.width/icon_res_x))
  f.write("const char icons_data[icons_length] = {\n")
  st=""
  first=True
  for pix in pix_val:
    # take only the alpha channel
    c = pix[3]
    if not first:
      st += ", "
    st += "0x%x" % c
    if c>0:
      print(c)
    first = False
  f.write(st)
      
  f.write("};\n")
