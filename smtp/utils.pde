void printArray(Print *output, char* delimeter, byte* data, int len, int base)
{
  char buf[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for(int i = 0; i < len; i++)
  {
    if(i != 0)
      output->print(delimeter);
    output->print(itoa(data[i], buf, base));
  }
}
