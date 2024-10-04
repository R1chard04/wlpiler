int containsDuplicates(int *array, int size) {
  int *seen = NULL;
  int i = 0;
  int theMax = 2147483467;
  int result = 0;
  theMax = max(array,size);
  seen = new int[theMax];
  while(theMax >= i+1) {
    *(seen+i) = 0;
    i = i+1;
  }
  i = 0;
  while(i+1 <= size) {
    if(*(seen+*(array+i)) == 1) {
      result = 1;
      println(*(array+i));
    } else {
      // not a duplicate :O
    }
    *(seen+*(array+i)) += 1;
    i = i + 1
  }
  delete [] seen;
  return result;
}
