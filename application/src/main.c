#include "binary_protocol.h"
#include "gpio_reaction.h"

int main(void)
{
    int initialization_result = initialize_gpio_reaction();

    if (0 != initialization_result)
    {
        return initialization_result;
    }

    return run_binary_protocol();
}
