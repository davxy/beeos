int __sync_lock_test_and_set_4(int *ptr, int val)
{
    // TODO atomic
    int old = *ptr;
    *ptr = val;
    return old;
}

void __sync_synchronize(void)
{
}
