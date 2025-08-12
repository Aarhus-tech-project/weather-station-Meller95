using Microsoft.EntityFrameworkCore;
using VejrstationApi.Data;

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.

// CORS
builder.Services.AddCors(o => o.AddDefaultPolicy(p =>
    p.WithOrigins("http://localhost:5173")
     .AllowAnyHeader()
     .AllowAnyMethod()));

builder.Services.AddControllers();
// Learn more about configuring Swagger/OpenAPI at https://aka.ms/aspnetcore/swashbuckle
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

// Configure Entity Framework Core with MySQL
var cs = builder.Configuration.GetConnectionString("VejrDb");
builder.Services.AddDbContext<VejrDbContext>(opts =>
    opts.UseMySql(cs, ServerVersion.AutoDetect(cs)));

var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

app.UseHttpsRedirection();

// CORS
app.UseCors();

app.UseAuthorization();

app.MapControllers();

app.Run();
