bool collission(int x1, int y1, int x2, int y2, int size1, int size2) {
  int w1 = size1;
  int h1 = size1;
  int w2 = size2;
  int h2 = size2;
  return ((abs(x1 - x2) * 2 < (w1 + w2)) && (abs(y1 - y2) * 2 < (h1 + h2)));
}

  
