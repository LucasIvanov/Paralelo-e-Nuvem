#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

/* ================= RANDOM ================= */
#define MODULUS    2147483647
#define MULTIPLIER 48271
#define DEFAULT    123456789

static long seed = DEFAULT;

double Random(void)
{
    const long Q = MODULUS / MULTIPLIER;
    const long R = MODULUS % MULTIPLIER;
    long t;

    t = MULTIPLIER * (seed % Q) - R * (seed / Q);
    if (t > 0)
        seed = t;
    else
        seed = t + MODULUS;

    return ((double) seed / MODULUS);
}

/* ================= ESTRUTURAS ================= */
typedef struct {
    double x, y, z;
    double mass;
} Particle;

typedef struct {
    double xold, yold, zold;
    double fx, fy, fz;
} ParticleV;

/* ================= PROTÓTIPOS ================= */
void InitParticles(Particle[], ParticleV[], int);
double ComputeForcesMPI(Particle[], Particle[], ParticleV[],
                        int, int, int);
double ComputeNewPos(Particle[], ParticleV[], int, double);

/* ================= MAIN ================= */
int main(int argc, char **argv)
{
    int rank, size;
    int npart, cnt;
    Particle  *particles;
    ParticleV *pv;
    double sim_t = 0.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        fscanf(stdin, "%d", &npart);
        fscanf(stdin, "%d", &cnt);
    }

    MPI_Bcast(&npart, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cnt,   1, MPI_INT, 0, MPI_COMM_WORLD);

    particles = (Particle *) malloc(sizeof(Particle) * npart);
    pv        = (ParticleV *) malloc(sizeof(ParticleV) * npart);

    if (rank == 0)
        InitParticles(particles, pv, npart);

    MPI_Bcast(particles, npart * sizeof(Particle), MPI_BYTE, 0, MPI_COMM_WORLD);
    MPI_Bcast(pv,        npart * sizeof(ParticleV), MPI_BYTE, 0, MPI_COMM_WORLD);

    double t_start = MPI_Wtime();

    while (cnt--) {
        double max_f_local, max_f_global;

        max_f_local = ComputeForcesMPI(particles, particles, pv,
                                       npart, rank, size);

        MPI_Allreduce(&max_f_local, &max_f_global, 1,
                      MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        sim_t += ComputeNewPos(particles, pv, npart, max_f_global);
    }

    double t_end = MPI_Wtime();

    if (rank == 0) {
        int i;
        for (i = 0; i < npart; i++)
            printf("%.5lf %.5lf %.5lf\n",
                   particles[i].x, particles[i].y, particles[i].z);

        printf("Tempo total MPI: %.6f segundos\n", t_end - t_start);
    }

    free(particles);
    free(pv);

    MPI_Finalize();
    return 0;
}

/* ================= FUNÇÕES ================= */

void InitParticles(Particle particles[], ParticleV pv[], int npart)
{
    int i;
    for (i = 0; i < npart; i++) {
        particles[i].x = Random();
        particles[i].y = Random();
        particles[i].z = Random();
        particles[i].mass = 1.0;

        pv[i].xold = particles[i].x;
        pv[i].yold = particles[i].y;
        pv[i].zold = particles[i].z;
        pv[i].fx = pv[i].fy = pv[i].fz = 0.0;
    }
}

double ComputeForcesMPI(Particle myp[], Particle others[],
                        ParticleV pv[], int npart,
                        int rank, int size)
{
    int i, j;
    double max_f = 0.0;

    int chunk = npart / size;
    int start = rank * chunk;
    int end   = (rank == size - 1) ? npart : start + chunk;

    for (i = start; i < end; i++) {
        double xi = myp[i].x;
        double yi = myp[i].y;
        double fx = 0.0, fy = 0.0;
        double rmin = 100.0;

        for (j = 0; j < npart; j++) {
            double rx = xi - others[j].x;
            double ry = yi - others[j].y;
            double r  = rx * rx + ry * ry;
            if (r == 0.0) continue;
            if (r < rmin) rmin = r;
            r = r * sqrt(r);
            fx -= others[j].mass * rx / r;
            fy -= others[j].mass * ry / r;
        }

        pv[i].fx += fx;
        pv[i].fy += fy;

        fx = sqrt(fx*fx + fy*fy) / rmin;
        if (fx > max_f) max_f = fx;
    }

    return max_f;
}

double ComputeNewPos(Particle particles[], ParticleV pv[],
                     int npart, double max_f)
{
    static double dt_old = 0.001, dt = 0.001;
    double a0, a1, a2, dt_new;
    int i;

    a0 = 2.0 / (dt * (dt + dt_old));
    a2 = 2.0 / (dt_old * (dt + dt_old));
    a1 = -(a0 + a2);

    for (i = 0; i < npart; i++) {
        double xi = particles[i].x;
        double yi = particles[i].y;

        particles[i].x = (pv[i].fx - a1*xi - a2*pv[i].xold) / a0;
        particles[i].y = (pv[i].fy - a1*yi - a2*pv[i].yold) / a0;

        pv[i].xold = xi;
        pv[i].yold = yi;
        pv[i].fx = pv[i].fy = 0.0;
    }

    dt_new = 1.0 / sqrt(max_f);
    if (dt_new < 1e-6) dt_new = 1e-6;

    if (dt_new < dt) {
        dt_old = dt;
        dt = dt_new;
    } else if (dt_new > 4.0 * dt) {
        dt_old = dt;
        dt *= 2.0;
    }

    return dt_old;
}
