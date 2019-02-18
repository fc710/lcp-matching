#include "lcp-search.h"
#include "read_text.h"

static inline int min(int a, int b) {
    return a < b ? a : b;
}

static int
suffix_cmp(const char* text, const unsigned int pos, const unsigned int n,
        const char* pattern, const unsigned int m) {
    size_t size = min(m , n - pos + 1);
    return strncmp(pattern, text + pos, size);
}

static unsigned int
lcp2(const char* s1, const char* s2, const unsigned int len) {
    unsigned int l = 0;
    for (;l < len && s1[l] == s2[l]; l++) {}
    return l;
}

/* returns the position of the suffix array corresponding to a match,
   returns -1 if no such match exists
   */
static unsigned int
sa_search(const char* text, const int* sa, const unsigned int n,
        const unsigned int* llcp,const unsigned int* rlcp,
        const char* pattern, const unsigned int m) {
    unsigned int found = -1;
    if (suffix_cmp(text, sa[0], n, pattern, m) < 0 ||
            suffix_cmp(text, sa[n - 1], n, pattern, m) > 0)
        return -1;
    /* The pattern is not outside the realm of available suffixes.
       Let's search for an occurrence
       */
    unsigned int lp = 0;
    unsigned int rp = n - 1;
    while (lp <= rp) {
        unsigned int mp = lp + (rp - lp) / 2;
        int cmp = suffix_cmp(text, sa[mp], n, pattern, m);
        if (cmp == 0) {
            found = mp;
            break;
        }
        if (cmp < 0)
            rp = mp - 1;
        else
            lp = mp + 1;
    }
    return found;
}
static unsigned int
sa_search_1(const char* text, const int* sa, const unsigned int n,
        const unsigned int* llcp, const unsigned int* rlcp,
        const char* pattern, const unsigned int m) {
    unsigned int found = -1;
    if (suffix_cmp(text, sa[0], n, pattern, m) < 0 ||
            suffix_cmp(text, sa[n - 1], n, pattern, m) > 0)
        return -1;
    /* The pattern is not outside the realm of available suffixes.
       Let's search for an occurrence
       */
    unsigned int lp = 0;
    unsigned int rp = n - 1;
    unsigned int l = lcp2(text + sa[lp], pattern, m);
    unsigned int r = lcp2(text + sa[rp], pattern, m);
    unsigned int mlr = min(l, r);
    while (lp <= rp) {
        unsigned int mp = lp + (rp - lp) / 2;
        int cmp = suffix_cmp(text, sa[mp] + mlr, n - mlr, pattern + mlr, m - mlr);
        if (cmp == 0) {
            found = mp;
            break;
        }
        if (cmp < 0) {
            rp = mp - 1;
            r = mlr + lcp2(text + sa[rp] +mlr, pattern + mlr, min(m - mlr, n - mlr));
        } else {
            lp = mp + 1;
            l = mlr + lcp2(text + sa[lp] +mlr, pattern + mlr, min(m - mlr, n - mlr));
        }
        mlr = min(l, r);
    }
    return found;
}

//Implementation of sa_search_2 from sa-matching with lcp_lr
//this is actually accelerant 3
static unsigned int
sa_search_2_1(const char* text, const int* sa, const unsigned int n,
        const unsigned int* llcp,const unsigned int* rlcp,
			  const char* pattern, const unsigned int m) {
    unsigned int found = -1;
    if (suffix_cmp(text, sa[0], n, pattern, m) < 0 ||
            suffix_cmp(text, sa[n - 1], n, pattern, m) > 0)
        return -1;
    /* The pattern is not outside the realm of available suffixes.
       Let's search for an occurrence
       */
    unsigned int lp = 0;
    unsigned int rp = n - 1;
    unsigned int l = lcp2(text + sa[lp], pattern, m);
    unsigned int r = lcp2(text + sa[rp], pattern, m);
    unsigned int mp = 0;
	//test function
    unsigned int
        lcp_t(const unsigned int p1, const unsigned int p2) {
            return lcp2(text + sa[p1], text + sa[p2], m);
        }
    while (lp < rp - 1) {
        mp = lp + (rp - lp) / 2;
        //printf("lp: %d, mp: %d, rp: %d, l: %d, r: %d\n",lp, mp, rp, l, r);
        if (l > r)
            if (llcp[mp] > l) {
				// printf("lcp(l,m) > l %d, lcp[lp,mp]: %d\n",llcp[mp],lcp_t(lp,mp));
                lp = mp;
            } else if (llcp[mp] < l) {
				// printf("lcp(l,m) < l %d, lcp[lp,mp]: %d\n",llcp[mp],lcp_t(lp,mp));
                rp = mp;
                //r += lcp2(text + sa[rp] + r, pattern + r, min(m - r, n - r));
                r = llcp[mp];
            } else {
                //printf("lcp(l,m) == l %d, lcp[lp,mp]: %d\n",llcp[mp],lcp_t(lp,mp));
                /* lcp_t(lp, mp) == l */
                int cmp = suffix_cmp(text, sa[mp] + l, n - l, pattern + l, m - l);
                if (cmp == 0) {
                    found = mp;
                    //printf("lp: %d, mp: %d, rp: %d\n",lp,mp,rp);
                    break;
                }
                if (cmp < 0) {
                    rp = mp;
                    r += lcp2(text + sa[mp] + r, pattern + r, min(m - r, n - r));
                } else {
                    lp = mp;
                    l += lcp2(text + sa[mp] + l, pattern + l, min(m - l, n - l));
                }
            }
            else if (l < r)
                if (rlcp[mp] > r) {
					// printf("lcp(r,m) > r %d, lcp[rp,mp]: %d\n",rlcp[mp],lcp_t(rp,mp));
                    rp = mp;
                } else if (rlcp[mp] < r) {
					//  printf("lcp(r,m) < r %d, lcp[rp,mp]: %d\n",rlcp[mp],lcp_t(rp,mp));
                    lp = mp;
                    //l += lcp2(text + sa[lp] + r, pattern + l, min(m - l, n - l));
                    l = rlcp[mp];
                } else {
                    /* lcp_t(rp, lp) == r */
					// printf("lcp(r,m) == r %d, lcp[rp,mp]: %d\n",rlcp[mp],lcp_t(rp,mp));
                    int cmp = suffix_cmp(text, sa[mp] + r, n - r, pattern + r, m - r);
                    if (cmp == 0) {
                        found = mp;
						//   printf("lp: %d, mp: %d, rp: %d\n",lp,mp,rp);
                        break;
                    }
                    if (cmp < 0) {
                        rp = mp;
                        r += lcp2(text + sa[mp] + r, pattern + r, min(m - r, n - r));
                    } else {
                        lp = mp;
                        l += lcp2(text + sa[mp] + l, pattern + l, min(m - l, n - l));
                    }
                }
                else {
					// printf("l==r lcp(l,m) == l %d, lcp[lp,mp]: %d, lcp(lp,rp) %d\n",llcp[mp],lcp_t(lp,mp), lcp_t(lp,rp));
                    int cmp = suffix_cmp(text, sa[mp] + l, n - l, pattern + l, m - l);
                    if (cmp == 0) {
                        found = mp;
						//  printf("lp: %d, mp: %d, rp: %d\n",lp,mp,rp);
                        break;
                    }
                    if (cmp < 0) {
                        rp = mp;
                        r += lcp2(text + sa[mp] + l, pattern + l, min(m - l, n - l));
                    } else {
                        lp = mp;
                        l += lcp2(text + sa[mp] + l, pattern + l, min(m - l, n - l));
                    }
                }

    }
    if (found == -1) {
        if (suffix_cmp(text, sa[lp], n, pattern, m) == 0) found = lp;
        if (suffix_cmp(text, sa[rp], n, pattern, m) == 0) found = rp;
    }
	// printf("lp: %d, mp: %d, rp: %d\n",lp,mp,rp);
    return found;
}
//Same algorithm as sa_search_2_1, from Manber/Myers 'Suffix arrays: a new
//method for on-line string searches'
static unsigned int
sa_search_3_1(const char* text, const int* sa, const unsigned int n,
        const unsigned int* llcp, const unsigned int* rlcp, 
        const char* pattern, const unsigned int m){
    unsigned int found = -1;
    if (suffix_cmp(text, sa[0], n, pattern, m) < 0 ||
            suffix_cmp(text, sa[n - 1], n, pattern, m) > 0)
        return -1;
    /* The pattern is not outside the realm of available suffixes.
       Let's search for an occurrence
       */
    unsigned int lp = 0;
    unsigned int rp = n - 1;
    unsigned int l = lcp2(text + sa[lp], pattern, m);
    unsigned int r = lcp2(text + sa[rp], pattern, m);
    unsigned int tmp = 0;
    unsigned int mp = 0;
    while (lp < rp - 1) {
        mp = lp + (rp - lp) / 2;
        //printf("lp: %d, mp: %d, rp: %d, l: %d, r: %d\n",lp, mp, rp, l, r);
        if (l >= r){
            if ((sa[mp] + l < n && l < m) && llcp[mp] >= l){ 
                //printf("entered l: text at %d: %c\n",sa[mp]+l,text[sa[mp]+l]);
                tmp = l + lcp2(text + sa[mp] + l, pattern + l, min(m - l, n - l));
            }
            else
                tmp = llcp[mp];
        }
        else {
            if((sa[mp] + r< n && r < m) && rlcp[mp] >= r){
                //printf("entered r: text at %d: %c\n",sa[mp]+r,text[sa[mp]+r]);
                tmp = r + lcp2(text + sa[mp] + r, pattern + r, min(m - r, n - r));
            }
            else
                tmp = rlcp[mp];
        }
        if(tmp >= m ||(sa[mp]+tmp < n && pattern[tmp] <= text[sa[mp]+tmp])){
            //		printf("Setting r to cmp\n");
            rp = mp;
            r = tmp;
        }
        else{
            //	printf("Setting l to cmp\n");
            lp = mp;
            l = tmp;
        }
        //		printf("cmp: %d\n",tmp);
    }

    if (suffix_cmp(text, sa[rp], n, pattern, m) == 0)
        found = rp;
    else
        if (suffix_cmp(text, sa[lp], n, pattern, m) == 0) 
            found = lp;
    //printf("lp: %d, mp: %d, rp: %d\n",lp,mp,rp);
    return found;
}


	  /* Not really efficient, but saves space (doesn't require lcp 
	  because -llcp/rlcp have all the info)
	  It basically scans the bst where every node is identified by a triple (l, m, r).
	  Because only m is known, it needs to scan from root until a leaf is found
	  then evaluates the lower and upper bounds
	*/

void find_all_occurrences(unsigned int found, uint32_t n, uint32_t m,
						  const char* text, const int* sa,
						  const unsigned int* llcp, const unsigned int* rlcp){
	
		//p is m while searching the lower bound
		//pp is the actual lower bound
        unsigned int p = found, pp = found;
		//the first time the search starts it should look in lcp(l, m) ->llcp(m)
		//after that it should look lcp(m, r) first, and then lcp(l, m)
        bool first = true;
        if(p == n -1)
            --p;
        while(p > 0 && pp >= 0){
            //printf("p: %d\n", p);
            unsigned int llp = 0, rlp = n-1, mlp=(llp+rlp)/2;
            while( p != mlp){
                while(p < mlp){
                    rlp = mlp;
                    mlp = (llp+ mlp) /2;
                }
                while(p > mlp){
                    llp = mlp;
                    mlp = (rlp+mlp) /2;
                }
            }
            if (first && p == mlp && p -1 == llp){
                if(pp > 0 && llcp[p] >= m){
                    --pp;
                    first = false;
                }
                else
                    break;
            }
            else{
                if (!first && p ==mlp && p+1 ==rlp){
                    if(pp > 0 && rlcp[p] >= m)
                        --pp;
                    else 
                        break;
                }
                if(!first && p == mlp && p-1 == llp){
                    if(pp > 0 && llcp[p] >= m)
                        --pp;
                    else
                        break;
                }
            }
            first = false;
            --p;

        }
        //printf("p: %d\n",pp);
        unsigned int r = found , rr = found + 1;
        first = true;
        if (r == n -1)
            --r;
        while(r < n-1){
            //   printf("r: %d\n", r);
            unsigned int llp = 0, rlp = n-1, mlp=(llp+rlp)/2;
            while(r != mlp){
                while(r < mlp){
                    //printf("Ent.. r <\n");
                    rlp = mlp;
                    mlp = (llp+ mlp) /2;
                }
                while(r > mlp){
                    //printf("Ent.. r >\n");
                    llp = mlp;
                    mlp = (rlp+mlp) /2;
                }
            }
            if (first && r == mlp && r + 1 == rlp){
                if(rr < n && rlcp[r] >= m){
                    ++rr;
                    first = false;
                }
                else
                    break;
            }
            else {
                //  printf(" r %d, mlp %d, llp %d\n",r,mlp,llp);
                if(!first && r == mlp && r-1 == llp){
                    if(rr < n -1 && llcp[r] >= m)
                        ++rr;
                    else
                        break;
                }
                //    printf(" r %d, mlp %d, rlp %d\n",r,mlp,rlp);
                if (!first && r ==mlp && r+1 ==rlp){
                    if(rr < n -1 && rlcp[r] >= m)
                        ++rr;
                    else
                        break;
                }
                //     printf("rr: %d\n", rr);
            }
            first = false;
            ++r;
        }
        //    printf("pp %d, rr: %d\n", pp,rr);

        printf("Found %d occurences\n",rr-pp);
        //	for (unsigned int pos = lp; pos < rp; pos++) {
        char* x = strndupa(text + sa[pp], m);
        //printf("pos %d, rp %d\n",pos,rp);
        printf("Occurrence %s at position %d, lp: %d, rp: %d\n", x, sa[pp],pp,rr);
        //		}
    }

int main(int argc, char** argv){
    static struct gengetopt_args_info args_info;
    assert(cmdline_parser(argc, argv, &args_info) == 0);
    char* text = read_text(args_info.text_arg);
    //char* text = args_info.text_arg;
    //char* pattern = argv[2];
    char* pattern = args_info.pattern_orig;
    uint32_t m = strlen(pattern);
    uint32_t n = strlen(text);
    char* filename = basename(args_info.text_arg);
	// + 5 is extra space for the following concat
    size_t e =strlen(filename) + 5;
    char no_ext[e];
    file_no_extension(filename, no_ext);
	//third argument can't be hard-coded (eg. 5)
	//or the compiler sees strncat as strcat
    strncat(no_ext, ".bin", e - strlen(no_ext));
    FILE *read_ptr = fopen(no_ext, "rb");
    if(read_ptr  == NULL){
        fprintf(stderr, "%s: No file \"%s\" found.\nRun lcp-create \"%s\" first.\n",
                argv[0], no_ext, args_info.text_arg);
        exit(EXIT_FAILURE);
    }
    printf("Text size: %d\n",n);
    int* sa = malloc (n* sizeof(*sa));
    //unsigned int* lcp = malloc (n* sizeof(*lcp));
    unsigned int* llcp = malloc (n* sizeof(*llcp));
    unsigned int* rlcp = malloc (n* sizeof(*rlcp));
    fread(sa, sizeof(*sa),n,read_ptr);
    //fread(lcp, sizeof(*lcp),n,read_ptr);
    fread(llcp, sizeof(*llcp),n,read_ptr);
    fread(rlcp, sizeof(*rlcp),n,read_ptr);
    /*
       printf("---sa---\n");
       for(int i=0; i<n;++i)
       printf("%d ",sa[i]);
       printf("\n---lcp---\n");
       for(int i=0; i<n;++i)
       printf("%d ",lcp[i]);

       printf("\n---llcp---\n");
       for(int i=0; i<n;++i)
       printf("%d ",llcp[i]);
       printf("\n---rlcp---\n");
       for(int i=0; i<n;++i)
       printf("%d ",rlcp[i]);
       printf("\n");
       */
    fclose(read_ptr);
    unsigned int (*search_f)(const char*, const int*, const unsigned int,
            const unsigned int*,const unsigned int*, const char*, const unsigned int);

    printf("Accelerant type %d\n", args_info.accelerant_arg);
    if (args_info.accelerant_arg == 1)
        search_f = sa_search_1;
    else if (args_info.accelerant_arg == 2)
        search_f = sa_search_2_1;
    else if (args_info.accelerant_arg == 3)
        search_f = sa_search_3_1;
    else
        search_f = sa_search;

    unsigned int found = search_f(text, sa, n, llcp, rlcp, pattern, m);

	printf("found: %d\n",found);

	/*if found != -1 we have found at least one occurrence.
      Let's find all of them
	*/
    if (found != -1)
		find_all_occurrences(found, n, m, text, sa, llcp, rlcp);
	else
        printf("No match found\n");
	//original-finds occurrences with lcp
	/*  unsigned int lp = found;
            for (;lp < n && lcp[lp] >= m; --lp) {
        //	printf("lp %d: %d\n",lp,lcp[lp]);
        }
        unsigned int rp = found +1;
        for (;rp < n && lcp[rp] >= m; ++rp) {
        //	 printf("rp %d: %d\n",rp,lcp[rp]);
        }
        */


	free(sa);
    //free(lcp);
    free(rlcp);
    free(llcp);

    return 0;
}
