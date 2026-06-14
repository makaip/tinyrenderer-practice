from PIL import Image
import numpy as np
import cv2
import os
import time

filename = "zbuffer.tga"

last_mtime = None
cv2.namedWindow("fb", cv2.WINDOW_AUTOSIZE)

while True:
    try:
        mtime = os.path.getmtime(filename)

        if mtime != last_mtime:
            img = Image.open(filename).convert("RGBA")
            arr = np.array(img)
            bgr = cv2.cvtColor(arr, cv2.COLOR_RGBA2BGR)

            MIN_WIDTH = 200

            h, w = bgr.shape[:2]

            if w < MIN_WIDTH:
                scale = MIN_WIDTH / w
                new_h = int(h * scale)
                bgr = cv2.resize(
                    bgr,
                    (MIN_WIDTH, new_h),
                    interpolation=cv2.INTER_NEAREST
                )

            cv2.imshow("fb", bgr)

            print("Reloaded")
            last_mtime = mtime

    except Exception as e:
        print(e)

    key = cv2.waitKey(100) & 0xFF
    if key in (ord("q"), 27):
        break

cv2.destroyAllWindows()