Program MergeSort;

type
  vector = array of integer;

function merge_sort( arr : vector ) : vector;
var
  l,r : vector;
  i,j,c,mid : integer;
begin
  if( length(arr) > 1 ) then begin
    mid := trunc( length(arr) / 2 );
    SetLength( l , mid );
    SetLength( r , length(arr) - mid );
    for i := 0 to mid - 1 do begin
      l[i] := arr[i];
    end;
    for i := 0 to length(r) - 1 do begin
      r[i] := arr[i + mid];
    end;
    l := merge_sort(l);
    r := merge_sort(r);
    i := 0;
    j := 0;
    c := 0;
    while ( i < length(l) ) and ( j < length(r) ) do begin
      if l[i] < r[j] then begin
        arr[c] := l[i];
        i := 1 + i;
      end
      else begin
        arr[c] := r[j];
        j := 1 + j;
      end;
      c := 1 + c;
    end;
    while i < length(l) do begin
      arr[c] := l[i];
      i := 1 + i;
      c := 1 + c;
    end;
    while j < length(r) do begin
      arr[c] := r[j];
      j := 1 + j;
      c := 1 + c;
    end;
  end;
  merge_sort := arr;
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
  arr := merge_sort(arr);
  for i := 0 to length(arr) - 1 do begin
    write(arr[i] , ' ');
  end;
  writeln('');
end.
