program QuickSort;

type
  vector = array of integer;

procedure swap( var a,b : integer );
var
  temp : integer;
begin
  temp := a;
  a := b;
  b := temp;
end;

procedure quick_sort( low,high : integer ; var arr : vector );
var
  lm,pivot,i : integer;
begin
  if low < high then begin
    pivot := arr[high];
    lm := low;
    for i := low to high - 1 do begin
      if arr[i] < pivot then begin
        swap(arr[i] , arr[lm]);
        lm := 1 + lm;
      end;
    end;
    swap(arr[lm] , arr[high]);
    quick_sort(low , lm - 1 , arr);
    quick_sort(1 + lm , high , arr);
  end;
end;

var
  tam,i : integer;
  arr : vector;
begin
  read(tam);
  SetLength(arr , tam);
  for i := 0 to tam - 1 do begin
    read(arr[i]);
  end;
  quick_sort(0,tam - 1,arr);
  for i := 0 to tam - 1 do begin
    write(arr[i] , ' ');
  end;
  writeln('');
end.
