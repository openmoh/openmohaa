/*
===========================================================================
Copyright (C) 2012 su44

This file is part of md5_2_skX source code.

md5_2_skX source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

md5_2_skX source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with md5_2_skX source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// writetiki.c - .tik model definition export

/*
It should be noted that the normal file structure for
a tiki file is that the .TIK file resides in a directory
above the directory that contains the data. For example,
MoHAA's mp40.tik file reside in:

/models/weapons/

Whereas its actual data files (single skd file and several skc's) reside in:

/models/weapons/mp40/

*/

#include "md5_2_skX.h"

// this should be used only when writing a new TIKI file
char *extractTIKIDataPath(const char *tikiName) {
	static char out[MAX_TOOLPATH];
	strcpy(out,tikiName);
	stripExt(out);
	backSlashesToSlashes(out);
	if(out[0] && out[strlen(out)-1] != '/') {
		strcat(out,"/");
	}
	return out;
}

void writeTIKI(tModel_t *m,const char *outFName) {
	FILE *out;
	int i;
	char path[MAX_TOOLPATH];
	const char *skelmodel;
	const char *p;
	char surf[24];

	// path should be something like this:
	// "models/weapons/mp40"
	p = extractTIKIDataPath(outFName);
	p = getGamePath(p);
	if(p) {
		strcpy(path,p);
	} else {
		path[0] = 0;
	}
	skelmodel = getFName(outSKDMesh);

	T_Printf("Writing MoHAA .tik file %s...\n",outFName);

	out = F_Open(outFName,"w");
	if(out == 0) {
		T_Printf("Cannot open %s for writing\n",outFName);
		return;
	}

	fprintf(out,"//////////////////////////////////////////////////////////////////////////\n");
	fprintf(out,"//\n");
	fprintf(out,"// Exported by md5_2_skX\n");
	fprintf(out,"//\n");
	fprintf(out,"//////////////////////////////////////////////////////////////////////////\n");
fprintf(out,"\n");
	fprintf(out,"TIKI\n");
fprintf(out,"\n");
	fprintf(out,"setup\n");
	fprintf(out,"{\n");
		fprintf(out,"\tscale 1.0\n");
		fprintf(out,"\n");
		fprintf(out,"\tpath %s\n",path);
		fprintf(out,"\tskelmodel %s\n",skelmodel);
		for(i = 0; i < m->numSurfaces; i++) {
			// cap the len of surface name to 24,
			// longer surface names crashes MoHAA
			Q_strncpyz(surf,m->surfs[i].name,sizeof(surf));
			fprintf(out,"\tsurface %s shader %s\n",surf,m->surfs[i].name);
		}
		fprintf(out,"\n");
	fprintf(out,"}\n");
fprintf(out,"\n");
	fprintf(out,"init\n");
	fprintf(out,"{\n");
		fprintf(out,"\tserver\n");
		fprintf(out,"\t{\n");
			fprintf(out,"\t\tclassname Animate\n");
		fprintf(out,"\t}\n");
		fprintf(out,"\tclient\n");
		fprintf(out,"\t{\n");
			fprintf(out,"\t\t// setup emitters / cache images here...\n");
		fprintf(out,"\t}\n");
	fprintf(out,"}\n");
fprintf(out,"\n");
	fprintf(out,"animations\n");
	fprintf(out,"{\n");
		for(i = 0; i < numAnims; i++) {
			char alias[MAX_TOOLPATH];

			if(anims[i] == 0) {
				// something failed to load
				continue;
			}
			p = getFName(outSKCAnim[i]);

			strcpy(alias,p);
			stripExt(alias);

			fprintf(out,"\t%s %s\n",alias,p);
		}
	fprintf(out,"}\n");
fprintf(out,"\n");
	fclose(out);

	T_Printf("Wrote MoHAA TIKI file %s\n",outFName);
}


