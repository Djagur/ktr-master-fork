/^Process Time:/ { tail = 1 }
tail { print > (pre "_summary.txt"); next }
/^---|^<Pattern|^Exception Score|^pe-[0-9]+:| Exception[.]$/ { print > (pre "_top.txt") }
