#!/bin/bash
# Script outline to install and build kernel.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

OUTDIR=$(realpath "$OUTDIR")
mkdir -p ${OUTDIR}

if [ ! -d "${OUTDIR}" ]; then
    echo "Directory creation failed for ${OUTDIR}"
    exit 1
fi


cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
	
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j4 all
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/.

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

mkdir -p ${OUTDIR}/rootfs
cd "$OUTDIR"/rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    make distclean
    make defconfig
else
    cd busybox
    make distclean
    make defconfig
fi

make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
cd "$OUTDIR"/rootfs
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)

cp -a $SYSROOT/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
cp -a $SYSROOT/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64
cp -a $SYSROOT/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64
cp -a $SYSROOT/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64

sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1
sudo mknod -m 666 dev/tty c 5 0

cd ${FINDER_APP_DIR}
make clean
make CROSS_COMPILE=${CROSS_COMPILE}
 
cp -a ${FINDER_APP_DIR}/writer ${OUTDIR}/rootfs/home/.
cp -a ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home/.
cp -a ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home/.
cp -a ${FINDER_APP_DIR}/conf/assignment.txt ${OUTDIR}/rootfs/home/.
cp -a ${FINDER_APP_DIR}/conf/username.txt ${OUTDIR}/rootfs/home/.
cp -a ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home/.

cd "$OUTDIR"
sudo chown -R root:root rootfs/.

cd "$OUTDIR"/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
gzip -f ${OUTDIR}/initramfs.cpio
