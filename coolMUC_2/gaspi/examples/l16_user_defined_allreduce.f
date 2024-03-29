module my_reduce

      use gaspi_c_binding
      implicit none

contains

      function my_reduce_operation(op_one, op_two, op_res, &
&       op_state, num, element_size, timeout) &
&       result (res) bind(C, name="my_reduce_operation")
        implicit none
        integer(gaspi_number_t), intent(in), value :: num
        integer(c_int), intent(in) :: op_one(num)
        integer(c_int), intent(in) :: op_two(num)
        integer(c_int), intent(out) :: op_res(num)
        integer(c_int), intent(out) :: op_state(num)
        integer(gaspi_size_t), value :: element_size
        integer(gaspi_timeout_t), value :: timeout
        integer(gaspi_return_t) :: res
        integer i
        do i = 1, num
                op_res(i) = max(op_one(i), op_two(i))
        enddo
        res = GASPI_SUCCESS
      end function my_reduce_operation

end module my_reduce

program allreduce

      use gaspi_c_binding
      use my_reduce
      implicit none
      integer(gaspi_size_t) :: sizeof_int
      integer(gaspi_return_t) :: res
      integer(gaspi_rank_t) :: rank
      integer(c_int), dimension(i), target :: buffer_send
      integer(c_int), dimension(i), target :: buffer_recv
      integer(c_int), dimension(i), target :: reduce_state
      integer(gaspi_number_t) :: num_elem
      integer(gaspi_group_t) :: group
      integer(gaspi_timeout_t) :: timeout
      type(c_funptr) :: fproc

      sizeof_int = 4
      num_elem = 1
      group = GASPI_GROUP_ALL
      timeout = GASPI_BLOCK
      fproc = c_funloc(my_reduce_operation)
      res = gaspi_proc_init(timeout)
      res = gaspi_proc_rank(rank)

      buffer_send(1) = rank
      buffer_recv(1) = -1
      reduce_state(1) = 0
      res = gaspi_allreduce_user(C_LOC(buffer_send), &
&           C_LOC(buffer_recv), num_elem, sizeof_int, &
&           fproc, C_LOC(reduce_state), &
&           group, timeout)

        res = gaspi_proc_term(timeout)

end program allreduce
