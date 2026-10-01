C=======================================================================
C     TABLE77 - PRINT THE ATMOS77 TABLE EVERY 1000 M, 0 TO 20 KM.
C     OUTPUT IS CSV: H_M,T_K,P_PA,RHO_KGM3,A_MS
C=======================================================================
      PROGRAM TABLE77
      IMPLICIT NONE
      DOUBLE PRECISION H, T, P, RHO, A
      INTEGER I, IERR
C
      WRITE (*, '(A)') 'H_M,T_K,P_PA,RHO_KGM3,A_MS'
      DO 10 I = 0, 20
         H = 1000.0D0*DBLE(I)
         CALL ATMOS(H, T, P, RHO, A, IERR)
         IF (IERR .NE. 0) THEN
            WRITE (*, '(A,F9.1)') 'ERROR AT H = ', H
         ELSE
            WRITE (*, 100) H, T, P, RHO, A
         ENDIF
   10 CONTINUE
  100 FORMAT (F8.1, ',', F8.3, ',', F11.3, ',', F9.6, ',', F8.3)
      END
