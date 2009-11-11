!@+leo-ver=4-thin
!@+node:gcross.20091110205054.1939:@thin core.f95
!@@language fortran90
!@@tabwidth -2

!@+others
!@+node:gcross.20091110205054.1970:hello
subroutine hello
  print *, "Hello, world!"
end subroutine
!@-node:gcross.20091110205054.1970:hello
!@+node:gcross.20091110205054.1929:svd
function mysvd( &
  n, m, rank, &
  a, &
  u, s, vt &
) result (info)
  integer, intent(in) :: N, M, rank
  double complex, intent(in) :: a(n,m)
  double complex, intent(out) :: u(n,rank), vt(rank,m)
  double precision :: s(rank)

  double complex, allocatable :: work(:)
  integer :: iwork(8*rank)
  double precision :: rwork(5*rank*rank + 5*rank)
  double complex :: optimal_lwork
  integer :: lwork, info

  external :: zgesdd

  lwork = -1

  call zgesdd( &
    'S', n, m, &
    a, n, &
    s, &
    u, n, &
    vt, rank, &
    optimal_lwork, lwork, &
    rwork, &
    iwork, &
    info &
  )

  lwork = floor(real(optimal_lwork))

  allocate(work(lwork))

  call zgesdd( &
    'S', n, m, &
    a, n, &
    s, &
    u, n, &
    vt, rank, &
    work, lwork, &
    rwork, &
    iwork, &
    info &
  )

  deallocate(work)

end function
!@nonl
!@-node:gcross.20091110205054.1929:svd
!@+node:gcross.20091110205054.1940:Contractors
!@+node:gcross.20091110205054.1910:Main iteration
!@+node:gcross.20091106154604.1512:iteration_stage_1
subroutine iteration_stage_1( &
  bl, & ! state bandwidth dimension
  c, & ! operator bandwidth dimension
  d, & ! physical dimension
  left_environment, &
  number_of_matrices,sparse_operator_indices,sparse_operator_matrices, &
  iteration_stage_1_tensor &
)
  integer, intent(in) :: bl, c, d, number_of_matrices, sparse_operator_indices(2,number_of_matrices)
  double complex, intent(in) :: left_environment(bl,bl,c), sparse_operator_matrices(d,d,number_of_matrices)
  double complex, intent(out) :: iteration_stage_1_tensor(bl,d,c,bl,d)

  integer :: index, i, j, k1, k2
  double complex :: matrix(d,d)

  iteration_stage_1_tensor = 0

  do index = 1, number_of_matrices
    k1 = sparse_operator_indices(1,index)
    k2 = sparse_operator_indices(2,index)
    matrix  = sparse_operator_matrices(:,:,index)
    do j=1,bl
      do i=1,bl
        iteration_stage_1_tensor(i,:,k2,j,:) = iteration_stage_1_tensor(i,:,k2,j,:) + matrix(:,:)*left_environment(i,j,k1)
      end do
    end do
  end do

end subroutine
!@-node:gcross.20091106154604.1512:iteration_stage_1
!@+node:gcross.20091107163338.1529:iteration_stage_2
subroutine iteration_stage_2( &
  bl, & ! state left bandwidth dimension
  br, & ! state right bandwidth dimension
  c, &  ! operator bandwidth dimension
  d, &  ! physical dimension
  iteration_stage_1_tensor, &
  state_site_tensor, &
  iteration_stage_2_tensor &
)
  integer, intent(in) :: bl, br, c, d
  double complex, intent(in) :: state_site_tensor(br,bl,d), iteration_stage_1_tensor(bl,d,c,bl,d)
  double complex, intent(out) :: iteration_stage_2_tensor(br,c,bl,d)

  external :: zgemm

  call zgemm( &
      'N','N', &
      br,c*bl*d,bl*d, &
      (1d0,0d0), &
      state_site_tensor, br, &
      iteration_stage_1_tensor, bl*d, &
      (0d0,0d0), &
      iteration_stage_2_tensor, br &
  )

end subroutine
!@-node:gcross.20091107163338.1529:iteration_stage_2
!@+node:gcross.20091110011014.1551:iteration_stage_3
subroutine iteration_stage_3( &
  bl, & ! state left bandwidth dimension
  br, & ! state right bandwidth dimension
  c, &  ! operator bandwidth dimension
  d, &  ! physical dimension
  iteration_stage_2_tensor, &
  right_environment, &
  output_state_site_tensor &
)
  integer, intent(in) :: bl, br, c, d
  double complex, intent(in) :: right_environment(br,br,c), iteration_stage_2_tensor(br,c,bl,d)
  double complex, intent(out) :: output_state_site_tensor(br,bl,d)

  external :: zgemm

  call zgemm( &
      'N','N', &
      br, bl*d, br*c, &
      (1d0,0d0), &
      right_environment, br, &
      iteration_stage_2_tensor, br*c, &
      (0d0,0d0), &
      output_state_site_tensor, br &
  )

end subroutine
!@-node:gcross.20091110011014.1551:iteration_stage_3
!@-node:gcross.20091110205054.1910:Main iteration
!@+node:gcross.20091110205054.1911:Environment SOS contraction
!@+node:gcross.20091110011014.1549:contract_sos_left
subroutine contract_sos_left( &
  bl, & ! state left bandwidth dimension
  br, & ! state right bandwidth dimension
  c, &  ! operator bandwidth dimension
  d, &  ! physical dimension
  left_environment, &
  number_of_matrices,sparse_operator_indices,sparse_operator_matrices, &
  state_site_tensor, &
  new_left_environment &
)
  integer, intent(in) :: bl, br, c, d, number_of_matrices, sparse_operator_indices(2,number_of_matrices)
  double complex, intent(in) :: &
    left_environment(bl,bl,c), &
    state_site_tensor(br,bl,d), &
    sparse_operator_matrices(d,d,number_of_matrices)
  double complex, intent(out) :: new_left_environment(br,br,c)

  double complex :: &
    iteration_stage_1_tensor(bl,d,c,bl,d), &
    iteration_stage_2_tensor(br,c,bl,d), &
    iteration_stage_3_tensor(br,c,br)

  external :: zgemm

  ! Stage 1
  call iteration_stage_1( &
    bl, c, d, &
    left_environment, &
    number_of_matrices, sparse_operator_indices, sparse_operator_matrices, &
    iteration_stage_1_tensor &
  )
  ! Stage 2
  call iteration_stage_2( &
    bl, br, c, d, &
    iteration_stage_1_tensor, &
    state_site_tensor, &
    iteration_stage_2_tensor &
  )
  ! Stage 3
  call zgemm( &
      'N','C', &
      br*c,br,bl*d, &
      (1d0,0d0), &
      iteration_stage_2_tensor, br*c, &
      state_site_tensor, br, &
      (0d0,0d0), &
      iteration_stage_3_tensor, br*c &
  )
  ! Stage 4
  new_left_environment = reshape(iteration_stage_3_tensor,shape(new_left_environment),order=(/1,3,2/))

end subroutine
!@-node:gcross.20091110011014.1549:contract_sos_left
!@+node:gcross.20091110135225.1556:contract_sos_right_stage_1
subroutine contract_sos_right_stage_1( &
  bl, & ! state left bandwidth dimension
  br, & ! state right bandwidth dimension
  c, &  ! operator bandwidth dimension
  d, &  ! physical dimension
  right_environment, &
  state_site_tensor, &
  sos_right_stage_1_tensor &
)
  integer, intent(in) :: bl, br, c, d
  double complex, intent(in) :: &
    right_environment(br,br,c), &
    state_site_tensor(br,bl,d)
  double complex, intent(out) :: sos_right_stage_1_tensor(bl,d,br,c)

  external :: zgemm

  call zgemm( &
      'C','N', &
      bl*d, br*c, br, &
      (1d0,0d0), &
      state_site_tensor, br, &
      right_environment, br, &
      (0d0,0d0), &
      sos_right_stage_1_tensor, bl*d &
  )
end subroutine
!@-node:gcross.20091110135225.1556:contract_sos_right_stage_1
!@+node:gcross.20091110135225.1564:contract_sos_right_stage_2a
subroutine contract_sos_right_stage_2a( &
  bl, & ! state left bandwidth dimension
  br, & ! state right bandwidth dimension
  d, &  ! physical dimension
  matrix, &
  state_site_tensor, &
  sos_right_stage_2a_tensor &
)
  integer, intent(in) :: bl, br, d
  double complex, intent(in) :: &
    matrix(d,d), &
    state_site_tensor(br,bl,d)
  double complex, intent(out) :: sos_right_stage_2a_tensor(bl,d,br)

  integer :: i, j, k

  do j = 1,bl
  do i = 1,br
  do k = 1,d
    sos_right_stage_2a_tensor(j,k,i) = sum(state_site_tensor(i,j,:)*matrix(:,k))
  end do
  end do
  end do

end subroutine
!@-node:gcross.20091110135225.1564:contract_sos_right_stage_2a
!@+node:gcross.20091110135225.1570:contract_sos_right_stage_2b
subroutine contract_sos_right_stage_2b( &
  bl, & ! state left bandwidth dimension
  br, & ! state right bandwidth dimension
  d, &  ! physical dimension
  sos_right_stage_1_tensor_slice, &
  sos_right_stage_2a_tensor, &
  new_right_environment_slice &
)
  integer, intent(in) :: bl, br, d
  double complex, intent(in) :: &
    sos_right_stage_1_tensor_slice(bl,d,br), &
    sos_right_stage_2a_tensor(bl,d,br)
  double complex, intent(inout) :: new_right_environment_slice(bl,bl)

  external :: zgemm

  call zgemm( &
      'N','T', &
      bl, bl, d*br, &
      (1d0,0d0), &
      sos_right_stage_1_tensor_slice(1,1,1), bl, &
      sos_right_stage_2a_tensor, bl, &
      (1d0,0d0), &
      new_right_environment_slice(1,1), bl &
  )

end subroutine
!@-node:gcross.20091110135225.1570:contract_sos_right_stage_2b
!@+node:gcross.20091110135225.1572:contract_sos_right_stage_2
subroutine contract_sos_right_stage_2( &
  bl, & ! state left bandwidth dimension
  br, & ! state right bandwidth dimension
  c, &  ! operator bandwidth dimension
  d, &  ! physical dimension
  sos_right_stage_1_tensor, &
  number_of_matrices,sparse_operator_indices,sparse_operator_matrices, &
  state_site_tensor, &
  new_right_environment &
)
  integer, intent(in) :: bl, br, c, d, number_of_matrices, sparse_operator_indices(2,number_of_matrices)
  double complex, intent(in) :: &
    sos_right_stage_1_tensor(bl,d,br,c), &
    sparse_operator_matrices(d,d,number_of_matrices), &
    state_site_tensor(br,bl,d)
  double complex, intent(out) :: new_right_environment(bl,bl,c)

  integer :: index
  double complex :: &
    sos_right_stage_2a_tensor(bl,d,br)

  new_right_environment = 0

  do index = 1, number_of_matrices
    call contract_sos_right_stage_2a( &
      bl, br, d, &
      sparse_operator_matrices(:,:,index), &
      state_site_tensor, &
      sos_right_stage_2a_tensor &
    )
    call contract_sos_right_stage_2b( &
      bl, br, d, &
      sos_right_stage_1_tensor(1,1,1,sparse_operator_indices(2,index)), &
      sos_right_stage_2a_tensor, &
      new_right_environment(1,1,sparse_operator_indices(1,index)) &
    )
  end do

end subroutine
!@-node:gcross.20091110135225.1572:contract_sos_right_stage_2
!@+node:gcross.20091110205054.1907:contract_sos_right
subroutine contract_sos_right( &
  bl, & ! state left bandwidth dimension
  br, & ! state right bandwidth dimension
  c, &  ! operator bandwidth dimension
  d, &  ! physical dimension
  right_environment, &
  number_of_matrices,sparse_operator_indices,sparse_operator_matrices, &
  state_site_tensor, &
  new_right_environment &
)
  integer, intent(in) :: bl, br, c, d, number_of_matrices, sparse_operator_indices(2,number_of_matrices)
  double complex, intent(in) :: &
    right_environment(br,br,c), &
    state_site_tensor(br,bl,d), &
    sparse_operator_matrices(d,d,number_of_matrices)
  double complex, intent(out) :: new_right_environment(bl,bl,c)

  double complex :: &
    sos_right_stage_1_tensor(bl,d,br,c)

  call contract_sos_right_stage_1( &
    bl, br, c, d, &
    right_environment, &
    state_site_tensor, &
    sos_right_stage_1_tensor &
  )

  call contract_sos_right_stage_2( &
    bl, br, c, d, &
    sos_right_stage_1_tensor, &
    number_of_matrices,sparse_operator_indices,sparse_operator_matrices, &
    state_site_tensor, &
    new_right_environment &
  )

end subroutine
!@-node:gcross.20091110205054.1907:contract_sos_right
!@-node:gcross.20091110205054.1911:Environment SOS contraction
!@+node:gcross.20091110205054.1916:compute_expectation
function compute_expectation( &
  bl, & ! state left bandwidth dimension
  br, & ! state right bandwidth dimension
  c, &  ! operator bandwidth dimension
  d, &  ! physical dimension
  left_environment, &
  state_site_tensor, &
  number_of_matrices,sparse_operator_indices,sparse_operator_matrices, &
  right_environment &
) result (expectation)

  integer, intent(in) :: bl, br, c, d, number_of_matrices, sparse_operator_indices(2,number_of_matrices)
  double complex, intent(in) :: &
    left_environment(bl,bl,c), &
    state_site_tensor(br,bl,d), &
    right_environment(br,br,c), &
    sparse_operator_matrices(d,d,number_of_matrices)

  double complex :: new_right_environment(bl,bl,c), expectation
  integer :: i, j, k

  call contract_sos_right( &
    bl, br, c, d, &
    right_environment, &
    number_of_matrices,sparse_operator_indices,sparse_operator_matrices, &
    state_site_tensor, &
    new_right_environment &
  )

  expectation = 0

  do k = 1, c
  do i = 1, bl
  do j = 1, bl
    expectation = expectation + new_right_environment(i,j,k)*left_environment(j,i,k)
  end do
  end do
  end do

end function
!@-node:gcross.20091110205054.1916:compute_expectation
!@-node:gcross.20091110205054.1940:Contractors
!@+node:gcross.20091109182634.1537:optimize
function optimize( &
  bl, br, & ! state bandwidth dimension
  c, & ! operator bandwidth dimension
  d, & ! physical dimension
  left_environment, &
  number_of_matrices, sparse_operator_indices, sparse_operator_matrices, &
  right_environment, &
  which, &
  tol, &
  number_of_iterations, &
  result &
) result (info)
  integer, intent(in) :: bl, br, c, d, number_of_matrices, sparse_operator_indices(2,number_of_matrices)
  integer, intent(inout) :: number_of_iterations
  double complex, intent(in) :: &
    left_environment(bl,bl,c), &
    right_environment(br,br,c), &
    sparse_operator_matrices(d,d,number_of_matrices)
  double complex, intent(inout) :: &
    result(br,bl,d)
  character, intent(in) :: which*2
  double precision, intent(in) :: tol

!@<< Interface >>
!@+node:gcross.20091109182634.1538:<< Interface >>
interface
  !@  @+others
  !@+node:gcross.20091109182634.1540:znaupd
  subroutine znaupd &
      ( ido, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, &
        ipntr, workd, workl, lworkl, rwork, info )
      character :: bmat*1, which*2
      integer :: ido, info, ldv, lworkl, n, ncv, nev
      double precision :: tol
      integer :: iparam(11), ipntr(14)
      double complex :: resid(n), v(ldv,ncv), workd(3*n), workl(lworkl)
      double precision :: rwork(ncv)
  end subroutine
  !@nonl
  !@-node:gcross.20091109182634.1540:znaupd
  !@+node:gcross.20091109182634.1539:zneupd
  subroutine zneupd (rvec, howmny, select, d, z, ldz, sigma, &
                     workev, bmat, n, which, nev, tol, &
                     resid, ncv, v, ldv, iparam, ipntr, workd,  &
                     workl, lworkl, rwork, info)
        character  :: bmat, howmny, which*2
        logical    :: rvec
        integer    :: info, ldz, ldv, lworkl, n, ncv, nev
        double complex :: sigma
        Double precision :: tol
        integer    :: iparam(11), ipntr(14)
        logical    :: select(ncv)
        double precision :: rwork(ncv)
        double complex :: &
                  d(nev), resid(n), v(ldv,ncv), z(ldz, nev), &
                  workd(3*n), workl(lworkl), workev(2*ncv)

  end subroutine
  !@-node:gcross.20091109182634.1539:zneupd
  !@-others
end interface
!@-node:gcross.20091109182634.1538:<< Interface >>
!@nl

  integer, parameter :: nev = 1, ncv = 3

  double complex :: &
    iteration_stage_1_tensor(bl,d,c,bl,d), &
    iteration_stage_2_tensor(br,c,bl,d), &
    v(br*bl*d,ncv), &
    workd(3*br*bl*d), &
    workl(3*ncv**2+5*ncv), &
    eigenvalues(nev+1), &
    workev(2*ncv)

  integer :: &
    iparam(11), &
    ipntr(14), &
    ido, &
    info

  logical :: &
    select(ncv)

  double precision :: rwork(ncv)

!@+at
! First do the stage 1 contraction, since it is independent of the state site 
! tensor.
!@-at
!@@c

  call iteration_stage_1( &
    bl, c, d, &
    left_environment, &
    number_of_matrices, sparse_operator_indices, sparse_operator_matrices, &
    iteration_stage_1_tensor &
  )

!@+at
! Set up the solver.
!@-at
!@@c

  iparam = 0
  ido = 0
  info = 1

  iparam(1) = 1
  iparam(3) = number_of_iterations
  iparam(7) = 1

!@+at
! Run the iteration.
!@-at
!@@c

  do while (ido /= 99)
    call znaupd ( &
      ido, 'I', d*bl*br, which, nev, tol, result, ncv, v, br*bl*d, &
      iparam, ipntr, workd, workl, 3*ncv**2+5*ncv, rwork, info &
    ) 
    if (ido == -1 .or. ido == 1) then
      call iteration_stage_2( &
        bl, br, c, d, &
        iteration_stage_1_tensor, &
        workd(ipntr(1)), &
        iteration_stage_2_tensor &
      )
      call iteration_stage_3( &
        bl, br, c, d, &
        iteration_stage_2_tensor, &
        right_environment, &
        workd(ipntr(2)) &
      )
    end if
  end do

  number_of_iterations = iparam(3)

  if( info < 0 ) then
    return
  end if

!@+at
! Post-processing
!@-at
!@@c

  call zneupd (.true.,'A', select, eigenvalues, v, br*bl*d, (0d0,0d0), workev, &
              'I', d*bl*br, which, nev, tol, result, ncv, &
              v, br*bl*d, iparam, ipntr, workd, workl, 3*ncv**2+5*ncv, &
              rwork, info)

  result = reshape(v(:,1),shape(result))

end function
!@-node:gcross.20091109182634.1537:optimize
!@+node:gcross.20091110205054.1942:Randomization
!@+node:gcross.20091110205054.1921:seed_randomizer
subroutine seed_randomizer(seed)
  integer, intent(in) :: seed
  call srand(seed)
end subroutine
!@-node:gcross.20091110205054.1921:seed_randomizer
!@+node:gcross.20091110205054.1920:randomize_state_site_tensor
subroutine randomize_state_site_tensor(br, bl, d, state_site_tensor)
  integer, intent(in) :: br, bl, d
  double complex, intent(out) :: state_site_tensor(br,bl,d)

  integer :: i, j, k

  do i = 1, br
  do j = 1, bl
  do k = 1, d
    state_site_tensor(i,j,k) = rand()*(1d0,0d0) + rand()*(0d0,1d0)
  end do
  end do
  end do
end subroutine
!@-node:gcross.20091110205054.1920:randomize_state_site_tensor
!@+node:gcross.20091110205054.1922:rand_norm_state_site_tensor
function rand_norm_state_site_tensor(bl, br, d, state_site_tensor) result (info)
  integer, intent(in) :: bl, br, d
  double complex, intent(out) :: state_site_tensor(br,bl,d)

  double complex :: u(bl,bl), vt(bl,br*d)
  double precision :: s(bl)
  integer :: info

  double complex :: normalized_state_site_tensor(bl,br*d)

  external :: zgemm

  if (br*d < bl) then
    print *, "Not enough degrees of freedom to normalize."
    print *, br*d, "<", bl
    stop
  end if

  call randomize_state_site_tensor(bl, br, d, normalized_state_site_tensor)

  info = mysvd(bl,br*d,bl,normalized_state_site_tensor,u,s,vt)

  call zgemm( &
    'N','N', &
    bl,br*d,bl, &
    (1d0,0d0), &
    u, bl, &
    vt, bl, &
    (0d0,0d0), &
    normalized_state_site_tensor, bl &
  )

  state_site_tensor = reshape(normalized_state_site_tensor,shape(state_site_tensor),order=(/2,1,3/))

end function
!@-node:gcross.20091110205054.1922:rand_norm_state_site_tensor
!@-node:gcross.20091110205054.1942:Randomization
!@+node:gcross.20091110205054.1943:Normalization
!@+node:gcross.20091110205054.1926:norm_denorm_going_left
function norm_denorm_going_left( &
  bll,bl,br, &
  dl,d, &
  site_tensor_to_denormalize, &
  site_tensor_to_normalize, &
  denormalized_site_tensor, &
  normalized_site_tensor &
) result (info)
  integer, intent(in) :: bll, bl, br, dl, d
  double complex, intent(in) :: &
    site_tensor_to_denormalize(bl,bll,dl), &
    site_tensor_to_normalize(br,bl,d)
  double complex, intent(out) :: &
    denormalized_site_tensor(bl,bll,dl), &
    normalized_site_tensor(br,bl,d)
  double complex :: &
    denormalized_tensor_workspace(bl,bll,dl), &
    normalized_tensor_workspace(bl,br,d)

  double complex :: u(bl,bl), vt(bl,br*d)
  double precision :: s(bl)
  integer :: info, i, j

  external :: zgemm

  if (br*d < bl) then
    print *, "Not enough degrees of freedom to normalize."
    print *, br*d, "<", bl
    stop
  end if

  normalized_tensor_workspace = reshape(site_tensor_to_normalize,shape(normalized_tensor_workspace),order=(/2,1,3/))

  info = mysvd(bl,br*d,bl,normalized_tensor_workspace,u,s,vt)

  call zgemm( &
    'N','N', &
    bl,br*d,bl, &
    (1d0,0d0), &
    u, bl, &
    vt, bl, &
    (0d0,0d0), &
    normalized_tensor_workspace, bl &
  )

  normalized_site_tensor = reshape(normalized_tensor_workspace,shape(normalized_site_tensor),order=(/2,1,3/))

  u = conjg(u)

  call zgemm( &
    'C','N', &
    bl,bll*dl,bl, &
    (1d0,0d0), &
    u, bl, &
    site_tensor_to_denormalize, bl, &
    (0d0,0d0), &
    denormalized_tensor_workspace, bl &
  )

  forall (i=1:bll,j=1:dl) &
    denormalized_tensor_workspace(:,i,j) = denormalized_tensor_workspace(:,i,j) * s(:)

  call zgemm( &
    'N','N', &
    bl,bll*dl,bl, &
    (1d0,0d0), &
    u, bl, &
    denormalized_tensor_workspace, bl, &
    (0d0,0d0), &
    denormalized_site_tensor, bl &
  )

end function
!@-node:gcross.20091110205054.1926:norm_denorm_going_left
!@+node:gcross.20091110205054.1935:norm_denorm_going_right
function norm_denorm_going_right( &
  bl,br,brr, &
  d,dr, &
  site_tensor_to_normalize, &
  site_tensor_to_denormalize, &
  normalized_site_tensor, &
  denormalized_site_tensor &
) result (info)
  integer, intent(in) :: bl, br, brr, dr, d
  double complex, intent(in) :: &
    site_tensor_to_normalize(br,bl,d), &
    site_tensor_to_denormalize(brr,br,dr)
  double complex, intent(out) :: &
    normalized_site_tensor(br,bl,d), &
    denormalized_site_tensor(brr,br,dr)
  double complex :: &
    denormalized_tensor_workspace_1(br,brr,dr), &
    denormalized_tensor_workspace_2(br,brr,dr)

  double complex :: u(br,br), vt(br,bl*d)
  double precision :: s(br)
  integer :: info, i, j

  external :: zgemm

  if (bl*d < br) then
    print *, "Not enough degrees of freedom to normalize."
    print *, bl*d, "<", br
    stop
  end if

  info = mysvd(br,bl*d,br,site_tensor_to_normalize,u,s,vt)

  call zgemm( &
    'N','N', &
    br,bl*d,br, &
    (1d0,0d0), &
    u, br, &
    vt, br, &
    (0d0,0d0), &
    normalized_site_tensor, br &
  )

  denormalized_tensor_workspace_1 = reshape( &
    site_tensor_to_denormalize, &
    shape(denormalized_tensor_workspace_1), &
    order=(/2,1,3/) &
  )

  u = conjg(u)

  call zgemm( &
    'C','N', &
    br,brr*dr,br, &
    (1d0,0d0), &
    u, br, &
    denormalized_tensor_workspace_1, br, &
    (0d0,0d0), &
    denormalized_tensor_workspace_2, br &
  )

  forall (i=1:brr,j=1:dr) &
    denormalized_tensor_workspace_2(:,i,j) = denormalized_tensor_workspace_2(:,i,j) * s(:)

  call zgemm( &
    'N','N', &
    br,brr*dr,br, &
    (1d0,0d0), &
    u, br, &
    denormalized_tensor_workspace_2, br, &
    (0d0,0d0), &
    denormalized_tensor_workspace_1, br &
  )

  denormalized_site_tensor = reshape( &
    denormalized_tensor_workspace_1, &
    shape(denormalized_site_tensor), &
    order=(/2,1,3/) &
  )

end function
!@-node:gcross.20091110205054.1935:norm_denorm_going_right
!@-node:gcross.20091110205054.1943:Normalization
!@-others
!@nonl
!@-node:gcross.20091110205054.1939:@thin core.f95
!@-leo