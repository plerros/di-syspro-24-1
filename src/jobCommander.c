#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "configuration.h"

#include "llnode.h"
#include "array.h"
#include "fifopipe.h"
#include "packet.h"
#include <unistd.h>

int main()
{
	struct wopipe *pipe = NULL;
	wopipe_new(&pipe, PIPE_NAME);


	char str_lorem[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. \
Curabitur turpis velit, efficitur sit amet consectetur nec, porta congue tellus. \
Morbi in turpis ac erat tristique mollis. Fusce tincidunt orci vel nisl \
ullamcorper efficitur. Nam ac elit non nibh fringilla lacinia eget at purus. \
Morbi fermentum sit amet eros at imperdiet. Praesent porta enim ut tincidunt \
semper. Quisque pharetra, sem non ultricies pretium, ex libero malesuada est, \
quis porta felis erat id massa. Nullam at lorem eleifend, bibendum nunc id, \
lobortis ligula. Aliquam semper augue sit amet justo tempus, sit amet sodales dui \
pretium. Aliquam sed leo libero. Sed accumsan, ex non porta hendrerit, metus nibh \
sollicitudin odio, vitae molestie orci neque nec lacus. Donec ursus magna eget \
dolor commodo, sed mattis ipsum laoreet.";

	char str_exit[] = "exit";

	char *str = &str_lorem[0];

	for (int j = 3; j > 0; j--) {
		struct llnode *ll = NULL;
		llnode_new(&ll, sizeof(char), NULL);

		if (j == 1)
			str = &str_exit[0];

		printf("SENDING: %s\n", str);

		for (size_t i = 0; i < strlen(str) + 1; i++)
			llnode_add(&ll, &(str[i]));

		struct array *arr = NULL;
		array_new(&arr, ll);
		llnode_free(ll);

		struct packets *p = NULL;
		packets_new(&p);
		packets_pack(p, arr);

		packets_send(p, pipe);
		packets_free(p);

		array_free(arr);
	}
	wopipe_free(pipe);
	return 0;
}