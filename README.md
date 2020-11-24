# Threading Toy

I have used pthreads many times in the past for mostly personal projects or
having to debug Asterisk's bad implementation. While I'm not the foremost
expert on threading, I would consider myself "proficient." What I'm NOT
proficient in is modern C++ (C++11 and beyond).

This project is an attempt to use modern practices and implement something that
should be non-trivial but not ridiculously hard: a priority work queue.

# Supposed Functionality

Three priorities of queue: high, medium, low.

Three counts of threads: high, medium, low.

High priority threads can ONLY deal with high priority work. This ensures that
high priority work is hard to starve out.

Medium priority threads can handle high or medium. They SHOULD prioritize high
priority tasks, though hard guarantees are hard to make because concurrency.

Low priority threads can handle any tasks, but should prioritize high, then
medium, then low.

Getting the staggered priorities working in an efficient manner was the biggest
challenge. I am definitely interested in if I could do that part more simply,
or with more guarantees, or more performant. Right now I know that I am waking
up more threads than I need to, but I don't see an obvious way around it.

# Things I Need To Learn

## Probably Doing Well

By my own estimation, I think I understand these things and any mistakes are
likely to be relatively minor.

std::thread
std::mutex
std::lock

## Do Not Understand

These are things I am either not doing well OR haven't figured out where to use
them, but I feel like I NEED to learn them (though maybe this project isn't
ideal for doing so).

emplacement
copy/move semantics
