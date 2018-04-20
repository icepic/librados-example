/*
 * This is cut-n-pasted from ceph.com API programming
 * examples. Any lines I've added is BSD licensed as
 * per the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rados/librados.h>

int main (int argc, char **argv)
{

  /* Declare the cluster handle and required arguments. */
  uint64_t flags=NULL;
  rados_t cluster=NULL;
  rados_ioctx_t io=NULL;
  char *cluster_name = "ceph";
  char *user_name = "client.jj";
  char *poolname = "jj-test";

  /* Initialize the cluster handle with the "ceph" cluster name and the "client.admin" user */
  int err;
  err = rados_create2(&cluster, cluster_name, user_name, flags);

  if (err < 0) {
    fprintf(stderr, "%s: Couldn't create the cluster handle! %s\n", argv[0], strerror(-err));
    exit(EXIT_FAILURE);
  } else {
    printf("\nCreated a cluster handle.\n");
  }


  /* Read a Ceph configuration file to configure the cluster handle. */
  err = rados_conf_read_file(cluster, "/etc/ceph/ceph.conf");
  if (err < 0) {
    fprintf(stderr, "%s: cannot read config file: %s\n", argv[0], strerror(-err));
    exit(EXIT_FAILURE);
  } else {
    printf("\nRead the config file.\n");
  }

  /* Read command line arguments */
  err = rados_conf_parse_argv(cluster, argc, (const char **)argv);
  if (err < 0) {
    fprintf(stderr, "%s: cannot parse command line arguments: %s\n", argv[0], strerror(-err));
    exit(EXIT_FAILURE);
  } else {
    printf("\nRead the command line arguments.\n");
  }

  /* Connect to the cluster */
  err = rados_connect(cluster);
  if (err < 0) {
    fprintf(stderr, "%s: cannot connect to cluster: %s\n", argv[0], strerror(-err));
    exit(EXIT_FAILURE);
  } else {
    printf("\nConnected to the cluster.\n");
  }

  err = rados_ioctx_create(cluster, poolname, &io);
  if (err < 0) {
    fprintf(stderr, "%s: cannot open rados pool %s: %s\n", argv[0], poolname, strerror(-err));
    rados_shutdown(cluster);
    exit(EXIT_FAILURE);
  } else {
    printf("\nCreated I/O context.\n");
  }

  /* Write data to the cluster synchronously. */
  err = rados_write(io, "hw", "Hello World!", 12, 0);
  if (err < 0) {
    fprintf(stderr, "%s: Cannot write object \"hw\" to pool %s: %s\n", argv[0], poolname, strerror(-err));
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);
    exit(1);
  } else {
    printf("\nWrote \"Hello World\" to object \"hw\".\n");
  }

  char xattr[] = "en_US";
  err = rados_setxattr(io, "hw", "lang", xattr, 5);
  if (err < 0) {
    fprintf(stderr, "%s: Cannot write xattr to pool %s: %s\n", argv[0], poolname, strerror(-err));
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);
    exit(1);
  } else {
    printf("\nWrote \"en_US\" to xattr \"lang\" for object \"hw\".\n");
  }

  /*
   * Read data from the cluster asynchronously.
   * First, set up asynchronous I/O completion.
   */
  rados_completion_t comp;
  err = rados_aio_create_completion(NULL, NULL, NULL, &comp);
  if (err < 0) {
    fprintf(stderr, "%s: Could not create aio completion: %s\n", argv[0], strerror(-err));
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);
    exit(1);
  } else {
    printf("\nCreated AIO completion.\n");
  }

  /* Next, read data using rados_aio_read. */
  char read_res[100];
  err = rados_aio_read(io, "hw", comp, read_res, 12, 0);
  if (err < 0) {
    fprintf(stderr, "%s: Cannot read object. %s %s\n", argv[0], poolname, strerror(-err));
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);
    exit(1);
  } else {
    printf("\nComplete: %d", rados_aio_is_complete(comp));
    printf("\nRead object \"hw\". The contents are:\n %s \n", read_res);
  }

  /* Wait for the operation to complete */
  rados_aio_wait_for_complete(comp);
  printf("\nComplete: %d", rados_aio_is_complete(comp));
  printf("\nRead object \"hw\". The contents are:\n %s \n", read_res);

  /* Release the asynchronous I/O complete handle to avoid memory leaks. */
  rados_aio_release(comp);

  char xattr_res[100];
  err = rados_getxattr(io, "hw", "lang", xattr_res, 5);
  if (err < 0) {
    fprintf(stderr, "%s: Cannot read xattr. %s %s\n", argv[0], poolname, strerror(-err));
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);
    exit(1);
  } else {
    printf("\nRead xattr \"lang\" for object \"hw\". The contents are:\n %s \n", xattr_res);
  }

  err = rados_rmxattr(io, "hw", "lang");
  if (err < 0) {
    fprintf(stderr, "%s: Cannot remove xattr. %s %s\n", argv[0], poolname, strerror(-err));
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);
    exit(1);
  } else {
    printf("\nRemoved xattr \"lang\" for object \"hw\".\n");
  }

  err = rados_remove(io, "hw");
  if (err < 0) {
    fprintf(stderr, "%s: Cannot remove object. %s %s\n", argv[0], poolname, strerror(-err));
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);
    exit(1);
  } else {
    printf("\nRemoved object \"hw\".\n");
  }

  if (io) rados_ioctx_destroy(io);
  if (cluster) rados_shutdown(cluster);

  return 0;
}
