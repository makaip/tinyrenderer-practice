from PIL import Image
import numpy as np
import cv2
import os
import time

filename = "framebuffer.tga"
img = Image.open(filename).convert("RGBA")
img.save("output.png")