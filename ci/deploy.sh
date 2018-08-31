#!/bin/sh
SSH_PORT="10022"
SSH_HOST="aprsbuild@hamspirit.at"
SSH_PATH="/home/aprsbuild/public_html"

VER=`echo $CI_BUILD_REF | cut -b -7`
ARCHLIST=`ls | grep out-`

# create branch directory
BRANCHD=$SSH_PATH/branch-$CI_COMMIT_REF_NAME
ssh -p $SSH_PORT $SSH_HOST \
"mkdir -p $BRANCHD"

for i in $ARCHLIST; do
	OUTNAME=`echo $i | cut -f 2 -d '-'`
	echo -----------------------------------------------------------
	echo process $i ...
	# clean up artifacts
	rm -f $i/*.o
	rm -f $i/*.tgz
	cd $i
	# tgz the build output
	tar -czf \
	dxlAPRS_$OUTNAME-$VER.tgz \
	* ../aprsmap_common/* ../dxlAPRS_common/* ../scripts/*
	SSH_DEST=$SSH_HOST:$SSH_PATH/branch-$CI_COMMIT_REF_NAME
	# copy tgz to server
	echo "copy dxlAPRS_$OUTNAME-$VER.tgz to $SSH_DEST"
	scp -P $SSH_PORT dxlAPRS_$OUTNAME-$VER.tgz $SSH_DEST
	# create "current" links
	ssh -p $SSH_PORT $SSH_HOST \
	"rm -f $BRANCHD/dxlAPRS_$OUTNAME-current.tgz"
	ssh -p $SSH_PORT $SSH_HOST \
	"ln -s $BRANCHD/dxlAPRS_$OUTNAME-$VER.tgz $BRANCHD/dxlAPRS_$OUTNAME-current.tgz"
	cd ..
	# on master branch we create "current" links on html-root
	# for compatibility reasons
	if [ "$CI_COMMIT_REF_NAME" = "master" ]; then
		ssh -p $SSH_PORT $SSH_HOST \
		"rm -f $SSH_PATH/dxlAPRS_$OUTNAME-current.tgz"
		ssh -p $SSH_PORT $SSH_HOST \
		"ln -s $BRANCHD/dxlAPRS_$OUTNAME-$VER.tgz $SSH_PATH/dxlAPRS_$OUTNAME-current.tgz"
	fi
done

echo "source code from : $CI_PROJECT_URL" > revInfo-$VER.txt
echo "commit ID        : $CI_BUILD_REF" >> revInfo-$VER.txt
echo "title            : $CI_COMMIT_TITLE" >> revInfo-$VER.txt
echo $CI_COMMIT_DESCRIPTION >> revInfo-$VER.txt
echo $CI_COMMIT_MESSAGE >> revInfo-$VER.txt
scp -P $SSH_PORT revInfo-$VER.txt $SSH_DEST
scp -P $SSH_PORT revInfo-$VER.txt $SSH_DEST/revInfo_current.txt

