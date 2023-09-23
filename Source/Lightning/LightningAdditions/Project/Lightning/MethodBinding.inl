//BeginBound
template <typename FunctionType, FunctionType function>
static void BoundStatic(Call& call, ExceptionReport& report)
{
  function();
}
//EndBound
template <typename FunctionType, FunctionType function>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (*)() )
{
  BoundFn boundFunction = BoundStatic<FunctionType, function>;
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Arg0>
static void BoundStatic(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  function(arg0);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Arg0>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (*)(Arg0) )
{
  BoundFn boundFunction = BoundStatic<FunctionType, function, Arg0>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1>
static void BoundStatic(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  function(arg0, arg1);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (*)(Arg0, Arg1) )
{
  BoundFn boundFunction = BoundStatic<FunctionType, function, Arg0, Arg1>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2>
static void BoundStatic(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  function(arg0, arg1, arg2);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (*)(Arg0, Arg1, Arg2) )
{
  BoundFn boundFunction = BoundStatic<FunctionType, function, Arg0, Arg1, Arg2>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static void BoundStatic(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  function(arg0, arg1, arg2, arg3);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (*)(Arg0, Arg1, Arg2, Arg3) )
{
  BoundFn boundFunction = BoundStatic<FunctionType, function, Arg0, Arg1, Arg2, Arg3>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static void BoundStatic(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  function(arg0, arg1, arg2, arg3, arg4);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (*)(Arg0, Arg1, Arg2, Arg3, Arg4) )
{
  BoundFn boundFunction = BoundStatic<FunctionType, function, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static void BoundStatic(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  ::byte* arg5Ptr = call.GetArgumentPointer<LightningBindingType(Arg5)>(5);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  LightningBindingType(Arg5) arg5 = call.CastArgumentPointer<LightningBindingType(Arg5)>(arg5Ptr);
  function(arg0, arg1, arg2, arg3, arg4, arg5);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) )
{
  BoundFn boundFunction = BoundStatic<FunctionType, function, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static void BoundStatic(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  ::byte* arg5Ptr = call.GetArgumentPointer<LightningBindingType(Arg5)>(5);
  ::byte* arg6Ptr = call.GetArgumentPointer<LightningBindingType(Arg6)>(6);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  LightningBindingType(Arg5) arg5 = call.CastArgumentPointer<LightningBindingType(Arg5)>(arg5Ptr);
  LightningBindingType(Arg6) arg6 = call.CastArgumentPointer<LightningBindingType(Arg6)>(arg6Ptr);
  function(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) )
{
  BoundFn boundFunction = BoundStatic<FunctionType, function, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  DelegateParameter& p6 = parameters.PushBack();
  p6.ParameterType = LightningTypeId(Arg6);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Return>
static void BoundStaticReturn(Call& call, ExceptionReport& report)
{
  Return result = function();
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Return>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (*)() )
{
  BoundFn boundFunction = BoundStaticReturn<FunctionType, function, Return>;
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0>
static void BoundStaticReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  Return result = function(arg0);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (*)(Arg0) )
{
  BoundFn boundFunction = BoundStaticReturn<FunctionType, function, Return, Arg0>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1>
static void BoundStaticReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  Return result = function(arg0, arg1);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (*)(Arg0, Arg1) )
{
  BoundFn boundFunction = BoundStaticReturn<FunctionType, function, Return, Arg0, Arg1>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2>
static void BoundStaticReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  Return result = function(arg0, arg1, arg2);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (*)(Arg0, Arg1, Arg2) )
{
  BoundFn boundFunction = BoundStaticReturn<FunctionType, function, Return, Arg0, Arg1, Arg2>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static void BoundStaticReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  Return result = function(arg0, arg1, arg2, arg3);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (*)(Arg0, Arg1, Arg2, Arg3) )
{
  BoundFn boundFunction = BoundStaticReturn<FunctionType, function, Return, Arg0, Arg1, Arg2, Arg3>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static void BoundStaticReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  Return result = function(arg0, arg1, arg2, arg3, arg4);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (*)(Arg0, Arg1, Arg2, Arg3, Arg4) )
{
  BoundFn boundFunction = BoundStaticReturn<FunctionType, function, Return, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static void BoundStaticReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  ::byte* arg5Ptr = call.GetArgumentPointer<LightningBindingType(Arg5)>(5);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  LightningBindingType(Arg5) arg5 = call.CastArgumentPointer<LightningBindingType(Arg5)>(arg5Ptr);
  Return result = function(arg0, arg1, arg2, arg3, arg4, arg5);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) )
{
  BoundFn boundFunction = BoundStaticReturn<FunctionType, function, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static void BoundStaticReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  ::byte* arg5Ptr = call.GetArgumentPointer<LightningBindingType(Arg5)>(5);
  ::byte* arg6Ptr = call.GetArgumentPointer<LightningBindingType(Arg6)>(6);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  LightningBindingType(Arg5) arg5 = call.CastArgumentPointer<LightningBindingType(Arg5)>(arg5Ptr);
  LightningBindingType(Arg6) arg6 = call.CastArgumentPointer<LightningBindingType(Arg6)>(arg6Ptr);
  Return result = function(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) )
{
  BoundFn boundFunction = BoundStaticReturn<FunctionType, function, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  DelegateParameter& p6 = parameters.PushBack();
  p6.ParameterType = LightningTypeId(Arg6);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::Static
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class>
static void BoundInstance(Call& call, ExceptionReport& report)
{
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  (self->*function)();
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)() )
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class>;
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0>
static void BoundInstance(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  (self->*function)(arg0);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0) )
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1>
static void BoundInstance(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  (self->*function)(arg0, arg1);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1) )
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2>
static void BoundInstance(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  (self->*function)(arg0, arg1, arg2);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2) )
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static void BoundInstance(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  (self->*function)(arg0, arg1, arg2, arg3);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3) )
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static void BoundInstance(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  (self->*function)(arg0, arg1, arg2, arg3, arg4);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4) )
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static void BoundInstance(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  ::byte* arg5Ptr = call.GetArgumentPointer<LightningBindingType(Arg5)>(5);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  LightningBindingType(Arg5) arg5 = call.CastArgumentPointer<LightningBindingType(Arg5)>(arg5Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  (self->*function)(arg0, arg1, arg2, arg3, arg4, arg5);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) )
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static void BoundInstance(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  ::byte* arg5Ptr = call.GetArgumentPointer<LightningBindingType(Arg5)>(5);
  ::byte* arg6Ptr = call.GetArgumentPointer<LightningBindingType(Arg6)>(6);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  LightningBindingType(Arg5) arg5 = call.CastArgumentPointer<LightningBindingType(Arg5)>(arg5Ptr);
  LightningBindingType(Arg6) arg6 = call.CastArgumentPointer<LightningBindingType(Arg6)>(arg6Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  (self->*function)(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) )
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  DelegateParameter& p6 = parameters.PushBack();
  p6.ParameterType = LightningTypeId(Arg6);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Return>
static void BoundInstanceReturn(Call& call, ExceptionReport& report)
{
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  Return result = (self->*function)();
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Return>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)() )
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return>;
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0>
static void BoundInstanceReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  Return result = (self->*function)(arg0);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0) )
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1>
static void BoundInstanceReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  Return result = (self->*function)(arg0, arg1);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1) )
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2>
static void BoundInstanceReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  Return result = (self->*function)(arg0, arg1, arg2);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2) )
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static void BoundInstanceReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  Return result = (self->*function)(arg0, arg1, arg2, arg3);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2, Arg3) )
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static void BoundInstanceReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  Return result = (self->*function)(arg0, arg1, arg2, arg3, arg4);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4) )
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static void BoundInstanceReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  ::byte* arg5Ptr = call.GetArgumentPointer<LightningBindingType(Arg5)>(5);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  LightningBindingType(Arg5) arg5 = call.CastArgumentPointer<LightningBindingType(Arg5)>(arg5Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  Return result = (self->*function)(arg0, arg1, arg2, arg3, arg4, arg5);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) )
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}
//BeginBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static void BoundInstanceReturn(Call& call, ExceptionReport& report)
{
  ::byte* arg0Ptr = call.GetArgumentPointer<LightningBindingType(Arg0)>(0);
  ::byte* arg1Ptr = call.GetArgumentPointer<LightningBindingType(Arg1)>(1);
  ::byte* arg2Ptr = call.GetArgumentPointer<LightningBindingType(Arg2)>(2);
  ::byte* arg3Ptr = call.GetArgumentPointer<LightningBindingType(Arg3)>(3);
  ::byte* arg4Ptr = call.GetArgumentPointer<LightningBindingType(Arg4)>(4);
  ::byte* arg5Ptr = call.GetArgumentPointer<LightningBindingType(Arg5)>(5);
  ::byte* arg6Ptr = call.GetArgumentPointer<LightningBindingType(Arg6)>(6);
  if (report.HasThrownExceptions()) return;  
  LightningBindingType(Arg0) arg0 = call.CastArgumentPointer<LightningBindingType(Arg0)>(arg0Ptr);
  LightningBindingType(Arg1) arg1 = call.CastArgumentPointer<LightningBindingType(Arg1)>(arg1Ptr);
  LightningBindingType(Arg2) arg2 = call.CastArgumentPointer<LightningBindingType(Arg2)>(arg2Ptr);
  LightningBindingType(Arg3) arg3 = call.CastArgumentPointer<LightningBindingType(Arg3)>(arg3Ptr);
  LightningBindingType(Arg4) arg4 = call.CastArgumentPointer<LightningBindingType(Arg4)>(arg4Ptr);
  LightningBindingType(Arg5) arg5 = call.CastArgumentPointer<LightningBindingType(Arg5)>(arg5Ptr);
  LightningBindingType(Arg6) arg6 = call.CastArgumentPointer<LightningBindingType(Arg6)>(arg6Ptr);
  Class* self = (Class*)call.GetHandle(Call::This).Dereference();
  Return result = (self->*function)(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
  if (report.HasThrownExceptions()) return; call.Set<Return>(Call::Return, result);
}
//EndBound
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) )
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  DelegateParameter& p6 = parameters.PushBack();
  p6.ParameterType = LightningTypeId(Arg6);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)() const)
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class>;
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Arg0>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0) const)
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1) const)
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2) const)
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3) const)
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4) const)
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) const)
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const)
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  DelegateParameter& p6 = parameters.PushBack();
  p6.ParameterType = LightningTypeId(Arg6);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(void),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Return>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)() const)
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return>;
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0) const)
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1) const)
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2) const)
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2, Arg3) const)
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4) const)
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) const)
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}

template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromMethod(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const)
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = LightningTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = LightningTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = LightningTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = LightningTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = LightningTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = LightningTypeId(Arg5);
  DelegateParameter& p6 = parameters.PushBack();
  p6.ParameterType = LightningTypeId(Arg6);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    LightningTypeId(Return),
    FunctionOptions::None
  );
}
