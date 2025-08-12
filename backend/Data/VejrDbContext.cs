using Microsoft.EntityFrameworkCore;
using VejrstationApi.Models;

namespace VejrstationApi.Data;

public class VejrDbContext : DbContext
{
    public VejrDbContext(DbContextOptions<VejrDbContext> options) : base(options) { }

    public DbSet<Måling> Målinger => Set<Måling>();
}
