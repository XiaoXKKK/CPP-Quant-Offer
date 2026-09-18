export const categories = [
  {
    id: 'cpp',
    name: 'C++ 语言与工具链',
    areas: [
      'Modern C++',
      'STL / C++ Object Model',
      'Template / Compile Time',
      'Compiler / Linker / ABI',
    ],
  },
  {
    id: 'concurrency',
    name: '内存模型与并发',
    areas: ['C++ Memory Model', 'Multithreading / Atomic / Lock-Free'],
  },
  { id: 'systems', name: 'Linux 与操作系统', areas: ['Linux', 'Operating System'] },
  {
    id: 'network',
    name: '网络与 I/O',
    areas: ['Network Programming', 'TCP/IP', 'epoll / io_uring'],
  },
  {
    id: 'performance',
    name: '性能与低延迟',
    areas: ['CPU Cache / NUMA', 'Performance Engineering', 'Low-Latency Programming'],
  },
  {
    id: 'trading',
    name: '交易基础设施',
    areas: ['Market Data', 'Order Book', 'Matching Engine', 'Feed Handler', 'Trading System'],
  },
  { id: 'design', name: '系统设计与算法', areas: ['System Design', 'Algorithm Coding'] },
];
export const roles = [
  'C++ Developer',
  'Quant Developer',
  'Low-Latency C++ Developer',
  'Trading Infrastructure Engineer',
];
export const companyTypes = ['高频交易', '量化私募', 'Trading Firm'];
export const sections = [
  '30 秒面试回答',
  '核心概念',
  '原理深入',
  '数据结构/系统内部实现',
  'C++ runnable demo',
  '高频追问',
  '容易答错的点',
  '性能分析',
  'Quant/Low-Latency 场景',
  '相关专题',
  '分层面试题',
];
