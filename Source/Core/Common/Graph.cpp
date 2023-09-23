// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#ifdef UseMemoryDebugger
#  include /**/ "Allocations.hpp" //@ignore (for the compactor turning this into a single hpp/cpp)
#endif

#ifdef UseMemoryTracker
#  include /**/ "Allocations.hpp" //@ignore (for the compactor turning this into a single hpp/cpp)
#endif

namespace Plasma
{

void* plAllocate(size_t numberOfBytes)
{
#ifdef UseMemoryDebugger
  void* ptr = DebugAllocate(numberOfBytes, AllocationType_Direct, 4);
#elif UseMemoryTracker
  void* ptr = DebugAllocate(numberOfBytes, 4);
#else
  void* ptr = malloc(numberOfBytes);
#endif

  TracyAlloc(ptr, numberOfBytes);
	
  return ptr;
}

void plDeallocate(void* ptr)
{
  TracyFree(ptr);
#ifdef UseMemoryDebugger
  DebugDeallocate(ptr, AllocationType_Direct);
#elif UseMemoryTracker
  return DebugDeallocate(ptr);
#else
  return free(ptr);
#endif
}

const uint cStaticMemoryBufferSize = 5000;
::byte StaticMemoryGraphBuffer[cStaticMemoryBufferSize];
::byte* BufferLocation = StaticMemoryGraphBuffer;

MemPtr plStaticAllocate(size_t size)
{
  // Static Memory graph nodes and other static objects
  // are allocated from a fixed size buffer this allows them to have controlled
  // or optional initialization and prevents them from showing up in leaks
  ErrorIf(BufferLocation + size >= StaticMemoryGraphBuffer + cStaticMemoryBufferSize,
          "Allocated too many memory graph objects. Increase "
          "cStaticMemoryBufferSize.");
  ::byte* current = BufferLocation;
  BufferLocation += size;
  // DebugPrint("Max Static Memory %d\n", uint(BufferLocation -
  // StaticMemoryGraphBuffer));
  return current;
}

namespace Memory
{
Stats::Stats() : Allocations(0), Active(0), BytesAllocated(0), BytesDedicated(0), PeakAllocated(0)
{
}

#define VisitByName(name) vistor(#name, name)

template <typename Vistor>
void Stats::Visit(Vistor& vistor, size_t flags)
{
  if (flags & ShowActive)
    VisitByName(Active);

  if (flags & ShowCount)
    VisitByName(Allocations);

  if (flags & ShowBytes)
    VisitByName(BytesAllocated);

  if (flags & ShowDedicated)
    VisitByName(BytesDedicated);

  if (flags & ShowPeak)
    VisitByName(PeakAllocated);
}

#undef VisitByName

void Stats::Accumulate(const Stats& right)
{
  Allocations += right.Allocations;
  Active += right.Active;
  BytesAllocated += right.BytesAllocated;
  BytesDedicated += right.BytesDedicated;
  PeakAllocated += right.PeakAllocated;
}

Root* Root::RootGraph = nullptr;
Heap* Root::GloblHeap = nullptr;
Heap* Root::StaticHeap = nullptr;

void Shutdown()
{
  GetRoot()->CleanUp();
  GetRoot()->Shutdown();
}

void Root::Shutdown()
{
  // Only delete the root
  // the root graph node will delete all child graph
  // nodes and clean up memory.
  if (RootGraph != nullptr)
  {
    delete RootGraph;
    RootGraph = nullptr;
  }
}

void DumpMemoryDebuggerStats(cstr projectName)
{
#ifdef UseMemoryDebugger
  BuildVerySleepyStats_ActiveAllocations(projectName);
#elif UseMemoryTracker
  OutputActiveAllocations("MyProject", VerySleepy_0_90);
#endif
}

void Root::Initialize()
{
#ifdef UseMemoryDebugger
  InitializeMemory();
#endif

  if (RootGraph == nullptr)
  {
    RootGraph = new Root("Root", nullptr);
    StaticHeap = new Heap("Static", RootGraph);
    GloblHeap = new Heap("Global", RootGraph);
  }
}

Root* GetRoot()
{
  Root::Initialize();
  return Root::RootGraph;
}

Heap* GetGlobalHeap()
{
  Root::Initialize();
  return Root::GloblHeap;
}

Heap* GetStaticHeap()
{
  Root::Initialize();
  return Root::StaticHeap;
}

const size_t maxTabs = 10;
const size_t tabSize = 2;

void Root::PrintAll()
{
  if (RootGraph)
    Root::RootGraph->PrintGraph(Stats::ShowBytes | Stats::ShowTotal | Stats::ShowActive);
}

class VistPrinter
{
public:
  void operator()(cstr /*name*/, MemCounterType var)
  {
    DebugPrint("%16u", var);
  }
};

class VistNamePrinter
{
public:
  void operator()(cstr name, MemCounterType /*var*/)
  {
    DebugPrint("%16s", name);
  }
};

Graph::Graph(cstr name, Graph* parent) : Name(name), mParent(parent)
{
  if (parent != nullptr)
    parent->Children.PushBack(this);
}

void Graph::PrintHeader(size_t flags)
{
  // DebugPrint("%-*s", maxTabs*tabSize, "Name" );

  VistNamePrinter p;

  if (flags & Stats::ShowLocal)
    mData.Visit(p, flags);

  if (flags & Stats::ShowTotal)
    mData.Visit(p, flags);

  DebugPrint("\n");
}

void Graph::CleanUp()
{
  InListBaseLink<Graph>::range sub = Children.All();
  while (!sub.Empty())
  {
    sub.Front().CleanUp();
    sub.PopFront();
  }
}

void Graph::PrintHelper(size_t tabs, size_t flags, cstr /*name*/)
{
  size_t tabWidth = tabs * tabSize;
  size_t nameWidth = (maxTabs - tabs) * tabSize;

  Stats total;
  this->Compute(total);

  DebugPrint("%*s%-*s", tabWidth, "", nameWidth, Name.c_str());
  VistPrinter p;

  if (flags & Stats::ShowLocal)
    mData.Visit(p, flags);

  if (flags & Stats::ShowTotal)
    total.Visit(p, flags);

  DebugPrint("\n");

  InListBaseLink<Graph>::range sub = Children.All();
  while (!sub.Empty())
  {
    sub.Front().Print(tabs + 1, flags);
    sub.PopFront();
  }
}

void Graph::Compute(Stats& data)
{
  data.Accumulate(mData);
  InListBaseLink<Graph>::range sub = Children.All();
  while (!sub.Empty())
  {
    sub.Front().Compute(data);
    sub.PopFront();
  }
}

void Graph::PrintGraph(size_t flags)
{
  PrintHeader(flags);
  Print(0, flags);
}

void Graph::Print(size_t tabs, size_t flags)
{
  PrintHelper(tabs, flags, "Main");
};

Graph::~Graph()
{
  DeleteObjectsIn(Children);
}

Heap* GetNamedHeap(cstr name)
{
  Root::Initialize();

  StringTokenRange tokens(name, Rune('.'));
  Graph* current = Root::RootGraph;
  Graph* parent = nullptr;
  StringRange token = name;
  while (!tokens.Empty() && current != nullptr)
  {
    parent = current;
    current = nullptr;

    InListBaseLink<Graph>::range managers = parent->Children.All();

    token = tokens.Front();
    while (!managers.Empty())
    {
      if (managers.Front().Name.c_str() == token)
        current = &managers.Front();
      managers.PopFront();
    }

    if (current == nullptr)
      current = new Heap(token.Data(), parent);

    tokens.PopFront();
  }

  return (Heap*)current;
}

} // namespace Memory

} // namespace Plasma
