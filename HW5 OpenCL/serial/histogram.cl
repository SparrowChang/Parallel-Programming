__kernel void histogram(__global unsigned int *image, __global unsigned int *results, unsigned int total_tasks, unsigned int task_per_thread) {
  // var
  int global_id = get_global_id(0);
  size_t global_size = get_global_size(0);

  if (global_id < 256 * 3)
    results[global_id] = 0;

  for (unsigned int i = 0; i < task_per_thread; i++) {
    if (global_id + i * global_size < total_tasks) {
      // 0: R, 1: G, 2: B
      for (unsigned int j = 0; j < 3; j++) {
        unsigned int index = image[(global_id + (i * global_size)) * 3 + j];
        atomic_inc(&results[index + j * 256]);
      }
    }
  }
}
